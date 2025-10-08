#include "ZmqServer.h"
#include <QDebug>
#include <QThread>
#include <QCoreApplication>

ZmqServer::ZmqServer(const QString& endpoint, QObject *parent)
    : QObject(parent)
    , m_endpoint(endpoint)
    , m_context(nullptr)
    , m_socket(nullptr)
    , m_studentManager(new StudentManager())
    , m_workerThread(new QThread(this))
    , m_running(false)
{
}

ZmqServer::~ZmqServer()
{
    stop();
}

void ZmqServer::start()
{
    try {
        m_context = new zmq::context_t(1);
        m_socket = new zmq::socket_t(*m_context, ZMQ_PUB);
        m_socket->bind(m_endpoint.toStdString());
        
        QStringList files = {"student_file_1.txt", "student_file_2.txt"};
        m_studentManager->loadStudentsFromFiles(files);
        
        m_running = true;
        
        m_workerThread->start();
        QMetaObject::invokeMethod(this, &ZmqServer::sendStudents, Qt::QueuedConnection);
        
        qDebug() << "ZMQ Server started on" << m_endpoint;
        
    } catch (const zmq::error_t& e) {
        qCritical() << "ZMQ error:" << e.what();
    }
}

void ZmqServer::stop()
{
    m_running = false;
    
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait(1000);
    }
    
    if (m_socket) {
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }
    
    if (m_context) {
        m_context->close();
        delete m_context;
        m_context = nullptr;
    }
}

void ZmqServer::sendStudents()
{
    while (m_running) {
        try {
            QByteArray data = m_studentManager->serializeStudents();
            
            if (!data.isEmpty()) {
                zmq::message_t message(data.size());
                memcpy(message.data(), data.constData(), data.size());
                
                auto result = m_socket->send(message, zmq::send_flags::dontwait);
                
                if (result.has_value() && result.value() > 0) {
                    qDebug() << "Sent" << data.size() << "bytes with student data";
                } else {
                    qDebug() << "No subscribers connected";
                }
            }
            
            for (int i = 0; i < 30 && m_running; ++i) {
                QThread::msleep(100);
            }
            
        } catch (const zmq::error_t& e) {
            if (m_running) {
                qCritical() << "Error sending message:" << e.what();
            }
            break;
        }
    }
    
    qDebug() << "ZmqServer: sendStudents loop finished";
}
