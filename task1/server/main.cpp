#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "ZmqServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("Student Server");
    app.setApplicationVersion("1.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("ZMQ Student Server");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption endpointOption(
        {"e", "endpoint"},
        "ZMQ endpoint to bind to",
        "endpoint",
        "tcp://*:5555"
    );
    parser.addOption(endpointOption);
    parser.process(app);
    
    QString endpoint = parser.value(endpointOption);
    
    ZmqServer server(endpoint);
    server.start();
    
    qDebug() << "Server running. Press Ctrl+C to stop.";
    
    return app.exec();
}
