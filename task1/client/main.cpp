#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include "ZmqClient.h"

class StudentDisplay
{
public:
    static void displayStudents(const QList<Student>& students) {
        qDebug() << "\n=== Received students list ===";
        qDebug() << "Total students:" << students.size();
        qDebug() << "==============================";
        
        for (const Student& student : students) {
            qDebug().noquote() 
                << QString("%1: %2 (%3)")
                   .arg(student.id())
                   .arg(student.fullName())
                   .arg(student.birthDate().toString("dd.MM.yyyy"));
        }
        qDebug() << "==============================\n";
    }
    
    static void displayError(const QString& error) {
        qCritical() << "Error:" << error;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("Student Client");
    app.setApplicationVersion("1.0");
    
    qRegisterMetaType<QList<Student>>("QList<Student>");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("ZMQ Student Client");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption endpointOption(
        {"e", "endpoint"},
        "ZMQ endpoint to connect to",
        "endpoint",
        "tcp://localhost:5555"
    );
    parser.addOption(endpointOption);
    parser.process(app);
    
    QString endpoint = parser.value(endpointOption);
    
    ZmqClient client(endpoint);
    
    QObject::connect(&client, &ZmqClient::studentsReceived, 
                     [](const QList<Student>& students) {
                         StudentDisplay::displayStudents(students);
                     });
    QObject::connect(&client, &ZmqClient::errorOccurred,
                     [](const QString& error) {
                         StudentDisplay::displayError(error);
                     });
    
    QTimer::singleShot(100, [&client]() {
        client.start();
    });
    
    qDebug() << "Client starting. Waiting for student data...";
    
    return app.exec();
}
