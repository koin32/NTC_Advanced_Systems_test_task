#ifndef COORDINATEPARSER_H
#define COORDINATEPARSER_H

#include <QVector>
#include <QRegularExpression>

struct Coordinate {
    double latitude;
    double longitude;
    QString originalText;
    QString context;
    QString name;
    bool isValid;
    
    Coordinate() : latitude(0.0), longitude(0.0), isValid(false) {}
};

class CoordinateParser
{
public:
    CoordinateParser();
    
    QVector<Coordinate> parseText(const QString& text);
    
private:
    struct CoordinateMatch {
        double lat;
        double lon;
        QString original;
        int start;
        int end;
    };
    
    QVector<CoordinateMatch> findAllCoordinates(const QString& text);
    CoordinateMatch parseCoordinateFormat(const QRegularExpressionMatch& match);
    double convertToDecimal(double degrees, double minutes = 0, double seconds = 0, 
                           const QString& hemisphere = "");
    bool validateCoordinate(double lat, double lon);
    QString extractContext(const QString& text, int start, int end);
    QString extractName(const CoordinateMatch& match, const QString& context);
    
    QVector<QRegularExpression> m_patterns;
    
    void initializePatterns();
};

#endif // COORDINATEPARSER_H
