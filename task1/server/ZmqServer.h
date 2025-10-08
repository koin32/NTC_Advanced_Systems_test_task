#ifndef ZMQSERVER_H
#define ZMQSERVER_H

#include <QObject>
#include <QThread>
#include <zmq.hpp>
#include "StudentManager.h"

class ZmqServer : public QObject
{
    Q_OBJECT

public:
    explicit ZmqServer(const QString& endpoint, QObject *parent = nullptr);
    ~ZmqServer();
    
    void start();
    void stop();
    
public slots:
    void sendStudents();

private:
    QString m_endpoint;
    zmq::context_t* m_context;
    zmq::socket_t* m_socket;
    StudentManager* m_studentManager;
    QThread* m_workerThread;
    bool m_running;
};

#endif // ZMQSERVER_H
