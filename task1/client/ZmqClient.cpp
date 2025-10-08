#include "ZmqClient.h"
#include <QDebug>
#include <QDataStream>
#include <QIODevice>
#include <QTimer>

ZmqClient::ZmqClient(const QString& endpoint, QObject *parent)
    : QObject(parent)
    , m_endpoint(endpoint)
    , m_context(nullptr)
    , m_socket(nullptr)
    , m_running(false)
{
    qRegisterMetaType<QList<Student>>("QList<Student>");
}

ZmqClient::~ZmqClient()
{
    stop();
}

void ZmqClient::start()
{
    try {
        m_context = new zmq::context_t(1);
        m_socket = new zmq::socket_t(*m_context, ZMQ_SUB);
        
        m_socket->set(zmq::sockopt::rcvtimeo, 100); 
        m_socket->set(zmq::sockopt::subscribe, ""); 
        
        qDebug() << "Connecting to:" << m_endpoint;
        m_socket->connect(m_endpoint.toStdString());
        
        m_running = true;
        
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ZmqClient::receiveStudents);
        timer->start(100); 
        
        qDebug() << "ZMQ Client connected to" << m_endpoint;
        
    } catch (const zmq::error_t& e) {
        qCritical() << "ZMQ error:" << e.what();
        emit errorOccurred(QString("ZMQ error: %1").arg(e.what()));
    }
}

void ZmqClient::stop()
{
    m_running = false;
    
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

void ZmqClient::receiveStudents()
{
    if (!m_running || !m_socket) return;
    
    try {
        zmq::message_t message;
        auto result = m_socket->recv(message, zmq::recv_flags::dontwait);
        
        if (result.has_value() && result.value() > 0) {
            QByteArray data(static_cast<char*>(message.data()), message.size());
            qDebug() << "Received" << data.size() << "bytes";
            
            QList<Student> students = deserializeStudents(data);
            
            if (!students.isEmpty()) {
                // Сортируем по фамилии
                std::sort(students.begin(), students.end());
                emit studentsReceived(students);
            } else {
                qDebug() << "Deserialized empty student list";
            }
        }
        
    } catch (const zmq::error_t& e) {
        if (m_running) {
            qCritical() << "Error receiving message:" << e.what();
            emit errorOccurred(QString("Receive error: %1").arg(e.what()));
        }
    }
}

QList<Student> ZmqClient::deserializeStudents(const QByteArray& data)
{
    QList<Student> students;
    
    if (data.isEmpty()) {
        qDebug() << "Empty data received";
        return students;
    }
    
    qDebug() << "Raw data size:" << data.size() << "bytes";
    qDebug() << "First 100 bytes hex:" << data.left(100).toHex();
    
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_15);
    
    // Проверяем статус потока перед чтением
    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Stream status error before reading:" << stream.status();
        return students;
    }
    
    int count = 0;
    stream >> count;
    
    qDebug() << "Trying to read student count, result:" << count;
    qDebug() << "Stream status after reading count:" << stream.status();
    
    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Error reading student count from stream, status:" << stream.status();
        return students;
    }
    
    if (count <= 0 || count > 1000) { 
        qWarning() << "Invalid student count:" << count;
        return students;
    }
    
    qDebug() << "Deserializing" << count << "students";
    
    for (int i = 0; i < count; ++i) {
        if (stream.atEnd()) {
            qWarning() << "Unexpected end of stream at student" << i;
            break;
        }
        
        int id;
        QString firstName, middleName, lastName;
        QDate birthDate;
        
        stream >> id >> firstName >> middleName >> lastName >> birthDate;
        
        qDebug() << "Read student" << i << ":" << id << firstName << middleName << lastName << birthDate.toString("dd.MM.yyyy");
        qDebug() << "Stream status:" << stream.status();
        
        if (stream.status() != QDataStream::Ok) {
            qWarning() << "Error reading student data at index" << i;
            break;
        }
        
        Student student(id, firstName, middleName, lastName, birthDate);
        if (student.isValid()) {
            students.append(student);
            qDebug() << "  ✓ Added:" << student.fullName();
        } else {
            qWarning() << "  ✗ Invalid student:" << id << firstName << middleName << lastName;
        }
    }
    
    qDebug() << "Successfully deserialized" << students.size() << "students";
    return students;
}
