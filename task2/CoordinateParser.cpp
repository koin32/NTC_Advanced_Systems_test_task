#include "CoordinateParser.h"
#include <QDebug>
#include <QStringList>
#include <cmath>

CoordinateParser::CoordinateParser()
{
    initializePatterns();
}

void CoordinateParser::initializePatterns()
{
    m_patterns.append(QRegularExpression(
        R"((-?\d{1,3}\.\d+)\s+(-?\d{1,3}\.\d+))"
    ));
    
    m_patterns.append(QRegularExpression(
        R"(([NS])(\d{1,3}\.\d+)\s+([WE])(\d{1,3}\.\d+))",
        QRegularExpression::CaseInsensitiveOption
    ));
    
    m_patterns.append(QRegularExpression(
        R"((\d{1,3})-(\d{1,2}(?:\.\d+)?)([NS])\s+(\d{1,3})-(\d{1,2}(?:\.\d+)?)([WE]))",
        QRegularExpression::CaseInsensitiveOption
    ));
    
    m_patterns.append(QRegularExpression(
        R"((\d{2})(\d{2})([NS])\s+(\d{2,3})(\d{2})([WE]))",
        QRegularExpression::CaseInsensitiveOption
    ));
    
    m_patterns.append(QRegularExpression(
        R"((\d{1,3})[°\s]*(\d{1,2}(?:\.\d+)?)?['\s]*([NS]?)[\s]*(\d{1,3})[°\s]*(\d{1,2}(?:\.\d+)?)?['\s]*([WE]?))",
        QRegularExpression::CaseInsensitiveOption
    ));
    
    m_patterns.append(QRegularExpression(
        R"((\d{1,3})[°\s]*(\d{1,2}(?:\.\d+)?)?['\s]*(с\.ш|ю\.ш|С|Ю)[\s]*(\d{1,3})[°\s]*(\d{1,2}(?:\.\d+)?)?['\s]*(в\.д|з\.д|В|З))",
        QRegularExpression::CaseInsensitiveOption
    ));
    
    m_patterns.append(QRegularExpression(
        R"((\d{1,3})[°\s]*(\d{1,2})['\s]*(\d{1,2}(?:\.\d+)?)?['\s]*([NS])[\s]*(\d{1,3})[°\s]*(\d{1,2})['\s]*(\d{1,2}(?:\.\d+)?)?['\s]*([WE]))",
        QRegularExpression::CaseInsensitiveOption
    ));
    
    m_patterns.append(QRegularExpression(
        R"((-?\d{1,3}[,.]\d+)[°\s]*[,;\s]+(-?\d{1,3}[,.]\d+)[°]?)"
    ));
}

QVector<Coordinate> CoordinateParser::parseText(const QString& text)
{
    QVector<Coordinate> result;
    QVector<CoordinateMatch> matches = findAllCoordinates(text);
    
    for (const CoordinateMatch& match : matches) {
        if (validateCoordinate(match.lat, match.lon)) {
            Coordinate coord;
            coord.latitude = match.lat;
            coord.longitude = match.lon;
            coord.originalText = match.original;
            coord.context = extractContext(text, match.start, match.end);
            coord.name = extractName(match, coord.context);
            coord.isValid = true;
            
            result.append(coord);
        }
    }
    
    return result;
}

QVector<CoordinateParser::CoordinateMatch> CoordinateParser::findAllCoordinates(const QString& text)
{
    QVector<CoordinateMatch> matches;
    
    for (const QRegularExpression& pattern : m_patterns) {
        QRegularExpressionMatchIterator it = pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            CoordinateMatch coordMatch = parseCoordinateFormat(match);
            if (coordMatch.lat != 0 || coordMatch.lon != 0) {
                coordMatch.start = match.capturedStart();
                coordMatch.end = match.capturedEnd();
                coordMatch.original = match.captured();
                matches.append(coordMatch);
            }
        }
    }
    
    return matches;
}

CoordinateParser::CoordinateMatch CoordinateParser::parseCoordinateFormat(const QRegularExpressionMatch& match)
{
    CoordinateMatch result;
    QString patternText = match.captured();

    if (patternText.contains(QRegularExpression(R"(-?\d+\.\d+\s+-?\d+\.\d+)"))) {
        result.lat = match.captured(1).toDouble();
        result.lon = match.captured(2).toDouble();
    }
    else if (patternText.contains(QRegularExpression(R"([NS]\d+\.\d+\s+[WE]\d+\.\d+)", 
                   QRegularExpression::CaseInsensitiveOption))) {
        QString latHemisphere = match.captured(1).toUpper();
        double latValue = match.captured(2).toDouble();
        QString lonHemisphere = match.captured(3).toUpper();
        double lonValue = match.captured(4).toDouble();
        
        result.lat = convertToDecimal(latValue, 0, 0, latHemisphere);
        result.lon = convertToDecimal(lonValue, 0, 0, lonHemisphere);
    }
    else if (patternText.contains(QRegularExpression(R"(\d+-\d+[NS]\s+\d+-\d+[WE])", 
                   QRegularExpression::CaseInsensitiveOption))) {
        double latDeg = match.captured(1).toDouble();
        double latMin = match.captured(2).toDouble();
        QString latHem = match.captured(3).toUpper();
        double lonDeg = match.captured(4).toDouble();
        double lonMin = match.captured(5).toDouble();
        QString lonHem = match.captured(6).toUpper();
        
        result.lat = convertToDecimal(latDeg, latMin, 0, latHem);
        result.lon = convertToDecimal(lonDeg, lonMin, 0, lonHem);
    }
    else if (patternText.contains("ю", Qt::CaseInsensitive) || 
             patternText.contains("з", Qt::CaseInsensitive)) {
        double lat = match.captured(1).toDouble();
        if (match.captured(3).contains("ю", Qt::CaseInsensitive)) {
            lat = -std::abs(lat);
        }
        
        double lon = match.captured(4).toDouble();
        if (match.captured(6).contains("з", Qt::CaseInsensitive)) {
            lon = -std::abs(lon);
        }
        
        result.lat = lat;
        result.lon = lon;
    }
    else {
        QRegularExpression numberRegex(R"(-?\d{1,3}(?:[.,]\d+)?)");
        QRegularExpressionMatchIterator numberIt = numberRegex.globalMatch(patternText);
        
        QVector<double> numbers;
        while (numberIt.hasNext() && numbers.size() < 4) {
            QString numStr = numberIt.next().captured().replace(',', '.');
            numbers.append(numStr.toDouble());
        }
        
        if (numbers.size() >= 2) {
            result.lat = numbers[0];
            result.lon = numbers[1];
            
            // Apply hemisphere corrections
            QString captured = patternText.toUpper();
            if (captured.contains('S') || captured.contains("Ю")) {
                result.lat = -std::abs(result.lat);
            }
            if (captured.contains('W') || captured.contains("З")) {
                result.lon = -std::abs(result.lon);
            }
        }
    }
    
    return result;
}

double CoordinateParser::convertToDecimal(double degrees, double minutes, double seconds, 
                                        const QString& hemisphere)
{
    double decimal = degrees + minutes / 60.0 + seconds / 3600.0;
    
    if (hemisphere == "S" || hemisphere == "W" || 
        hemisphere.contains("ю", Qt::CaseInsensitive) || 
        hemisphere.contains("з", Qt::CaseInsensitive)) {
        decimal = -decimal;
    }
    
    return decimal;
}

bool CoordinateParser::validateCoordinate(double lat, double lon)
{
    return (lat >= -90.0 && lat <= 90.0) && (lon >= -180.0 && lon <= 180.0);
}

QString CoordinateParser::extractContext(const QString& text, int start, int end)
{
    int contextStart = qMax(0, start - 100);
    int contextEnd = qMin(text.length(), end + 100);
    
    QString context = text.mid(contextStart, contextEnd - contextStart);
    
    QRegularExpression sentenceEnd(R"([.!?]\s)");
    QRegularExpressionMatch match;
    
    int firstEnd = context.indexOf(sentenceEnd, start - contextStart);
    if (firstEnd != -1) {
        contextEnd = contextStart + firstEnd + 1;
        context = text.mid(contextStart, contextEnd - contextStart);
    }
    
    int lastStart = context.lastIndexOf(sentenceEnd, start - contextStart);
    if (lastStart != -1) {
        contextStart = contextStart + lastStart + 1;
        context = text.mid(contextStart, contextEnd - contextStart);
    }
    
    return context.trimmed();
}

QString CoordinateParser::extractName(const CoordinateMatch& match, const QString& context)
{
    QRegularExpression namePattern(R"((?:точка|point|цель|target|угол|corner)\s+([A-Za-zА-Яа-я0-9]+))", 
                                  QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch nameMatch = namePattern.match(context);
    
    if (nameMatch.hasMatch()) {
        return nameMatch.captured(1);
    }
    
    return QString();
}
