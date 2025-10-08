#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "HttpServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("Coordinate Parser Service");
    app.setApplicationVersion("1.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("HTTP Service for Coordinate Parsing");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption portOption(
        {"p", "port"},
        "HTTP server port",
        "port",
        "8080"
    );
    parser.addOption(portOption);
    
    parser.process(app);
    
    quint16 port = parser.value(portOption).toUShort();
    
    HttpServer server(port);
    if (!server.start()) {
        qCritical() << "Failed to start HTTP server";
        return 1;
    }
    
    qDebug() << "Service running. Press Ctrl+C to stop.";
    
    return app.exec();
}
