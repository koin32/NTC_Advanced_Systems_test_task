#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include "CoordinateService.h"

class HttpServer : public QObject
{
    Q_OBJECT

public:
    explicit HttpServer(quint16 port = 8080, QObject *parent = nullptr);
    ~HttpServer();
    
    bool start();
    void stop();

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer* m_server;
    CoordinateService* m_coordinateService;
    quint16 m_port;
    
    void handleRequest(QTcpSocket* socket, const QByteArray& request);
    QByteArray createHttpResponse(const QJsonObject& data, int statusCode = 200);
    QByteArray createErrorResponse(const QString& error, int statusCode = 400);
};

#endif // HTTPSERVER_H
