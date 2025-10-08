#include "CoordinateService.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <cmath>

CoordinateService::CoordinateService()
{
}

QJsonObject CoordinateService::processText(const QString& text)
{
    QVector<Coordinate> coordinates = m_parser.parseText(text);
    
    QJsonObject result;
    result["total_coordinates"] = coordinates.size();
    result["geometry_type"] = determineGeometryType(coordinates);
    result["coordinates"] = coordinatesToJson(coordinates);
    
    return result;
}

QString CoordinateService::determineGeometryType(const QVector<Coordinate>& coordinates)
{
    if (coordinates.isEmpty()) {
        return "none";
    }
    
    if (coordinates.size() == 1) {
        return "point";
    }
    
    if (coordinates.size() >= 3) {
        const Coordinate& first = coordinates.first();
        const Coordinate& last = coordinates.last();
        
        double latDiff = std::abs(first.latitude - last.latitude);
        double lonDiff = std::abs(first.longitude - last.longitude);
        
        // Consider it closed if points are within 0.001 degrees (~100m)
        if (latDiff < 0.001 && lonDiff < 0.001) {
            return "polygon";
        }
        
        // Check if all points form a closed loop (approximate)
        bool isClosed = true;
        for (int i = 1; i < coordinates.size(); i++) {
            const Coordinate& prev = coordinates[i-1];
            const Coordinate& curr = coordinates[i];
            
            // Simple distance check - in real app use proper haversine formula
            double dist = std::sqrt(std::pow(prev.latitude - curr.latitude, 2) + 
                                  std::pow(prev.longitude - curr.longitude, 2));
            if (dist > 1.0) { // More than ~1 degree apart
                isClosed = false;
                break;
            }
        }
        
        if (isClosed) {
            return "polygon";
        }
    }
    
    return "line";
}

QJsonArray CoordinateService::coordinatesToJson(const QVector<Coordinate>& coordinates)
{
    QJsonArray jsonArray;
    
    for (const Coordinate& coord : coordinates) {
        QJsonObject jsonCoord;
        jsonCoord["latitude"] = coord.latitude;
        jsonCoord["longitude"] = coord.longitude;
        jsonCoord["original_text"] = coord.originalText;
        jsonCoord["context"] = coord.context;
        jsonCoord["name"] = coord.name;
        jsonCoord["is_valid"] = coord.isValid;
        
        jsonArray.append(jsonCoord);
    }
    
    return jsonArray;
}
