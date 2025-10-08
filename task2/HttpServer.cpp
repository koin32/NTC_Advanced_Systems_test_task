#include "HttpServer.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>

HttpServer::HttpServer(quint16 port, QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_coordinateService(new CoordinateService())
    , m_port(port)
{
}

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::start()
{
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        qCritical() << "Failed to start HTTP server on port" << m_port << ":" << m_server->errorString();
        return false;
    }
    
    connect(m_server, &QTcpServer::newConnection, this, &HttpServer::onNewConnection);
    
    qDebug() << "HTTP Server started on port" << m_port;
    qDebug() << "Available endpoints:";
    qDebug() << "  GET  /         - Service info";
    qDebug() << "  GET  /health   - Health check";
    qDebug() << "  POST /coordinates - Parse coordinates from text";
    
    return true;
}

void HttpServer::stop()
{
    if (m_server && m_server->isListening()) {
        m_server->close();
    }
}

void HttpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &HttpServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &HttpServer::onDisconnected);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        
        qDebug() << "New connection from:" << socket->peerAddress().toString();
    }
}

void HttpServer::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QByteArray requestData = socket->readAll();
    handleRequest(socket, requestData);
}

void HttpServer::onDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        qDebug() << "Client disconnected:" << socket->peerAddress().toString();
    }
}

void HttpServer::handleRequest(QTcpSocket* socket, const QByteArray& request)
{
    QString requestStr = QString::fromUtf8(request);
    QStringList lines = requestStr.split("\r\n");
    
    if (lines.isEmpty()) {
        socket->write(createErrorResponse("Invalid request"));
        return;
    }
    
    QString requestLine = lines[0];
    QStringList requestParts = requestLine.split(" ");
    if (requestParts.size() < 2) {
        socket->write(createErrorResponse("Invalid request line"));
        return;
    }
    
    QString method = requestParts[0];
    QString path = requestParts[1];
    
    qDebug() << "Request:" << method << path;
    
    if (method == "GET") {
        if (path == "/" || path == "/info") {
            QJsonObject response;
            response["service"] = "Coordinate Parser";
            response["version"] = "1.0";
            response["endpoints"] = QJsonArray::fromStringList({"/coordinates", "/health", "/info"});
            
            socket->write(createHttpResponse(response));
        }
        else if (path == "/health") {
            QJsonObject response;
            response["status"] = "healthy";
            response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            
            socket->write(createHttpResponse(response));
        }
        else {
            socket->write(createErrorResponse("Endpoint not found", 404));
        }
    }
    else if (method == "POST") {
        if (path == "/coordinates") {
            QString body;
            bool inBody = false;
            
            for (const QString& line : lines) {
                if (line.isEmpty()) {
                    inBody = true;
                    continue;
                }
                if (inBody) {
                    body = line;
                    break;
                }
            }
            
            if (body.isEmpty()) {
                socket->write(createErrorResponse("Empty request body"));
                return;
            }
            
            // Parse JSON
            QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8());
            if (doc.isNull() || !doc.isObject()) {
                socket->write(createErrorResponse("Invalid JSON in request body"));
                return;
            }
            
            QJsonObject requestObj = doc.object();
            if (!requestObj.contains("text") || !requestObj["text"].isString()) {
                socket->write(createErrorResponse("Missing or invalid 'text' field in request"));
                return;
            }
            
            QString text = requestObj["text"].toString();
            if (text.isEmpty()) {
                socket->write(createErrorResponse("Text cannot be empty"));
                return;
            }
            
            try {
                QJsonObject result = m_coordinateService->processText(text);
                socket->write(createHttpResponse(result));
            }
            catch (const std::exception& e) {
                qCritical() << "Error processing request:" << e.what();
                socket->write(createErrorResponse("Internal server error", 500));
            }
        }
        else {
            socket->write(createErrorResponse("Endpoint not found", 404));
        }
    }
    else {
        socket->write(createErrorResponse("Method not allowed", 405));
    }
    
    socket->disconnectFromHost();
}

QByteArray HttpServer::createHttpResponse(const QJsonObject& data, int statusCode)
{
    QJsonDocument doc(data);
    QByteArray jsonData = doc.toJson();
    
    QString response = QString(
        "HTTP/1.1 %1 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %2\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n"
        "%3"
    ).arg(statusCode).arg(jsonData.size()).arg(QString::fromUtf8(jsonData));
    
    return response.toUtf8();
}

QByteArray HttpServer::createErrorResponse(const QString& error, int statusCode)
{
    QJsonObject errorObj;
    errorObj["error"] = error;
    errorObj["status_code"] = statusCode;
    
    return createHttpResponse(errorObj, statusCode);
}
