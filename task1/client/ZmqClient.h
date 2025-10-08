#ifndef ZMQCLIENT_H
#define ZMQCLIENT_H

#include <QObject>
#include <QList>
#include <zmq.hpp>
#include "Student.h"

class ZmqClient : public QObject
{
    Q_OBJECT

public:
    explicit ZmqClient(const QString& endpoint, QObject *parent = nullptr);
    ~ZmqClient();
    
    void start();
    void stop();
    
signals:
    void studentsReceived(const QList<Student>& students);
    void errorOccurred(const QString& error);

private slots:
    void receiveStudents();

private:
    QString m_endpoint;
    zmq::context_t* m_context;
    zmq::socket_t* m_socket;
    bool m_running;
    
    QList<Student> deserializeStudents(const QByteArray& data);
};

#endif // ZMQCLIENT_H
