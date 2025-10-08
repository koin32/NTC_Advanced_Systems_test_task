#ifndef COORDINATESERVICE_H
#define COORDINATESERVICE_H

#include <QObject>
#include <QVector>
#include <QJsonObject>
#include "CoordinateParser.h"

class CoordinateService
{
public:
    CoordinateService();
    
    QJsonObject processText(const QString& text);
    
private:
    CoordinateParser m_parser;
    
    QString determineGeometryType(const QVector<Coordinate>& coordinates);
    QJsonArray coordinatesToJson(const QVector<Coordinate>& coordinates);
};

#endif // COORDINATESERVICE_H
