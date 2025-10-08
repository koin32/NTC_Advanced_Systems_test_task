#ifndef STUDENTPARSER_H
#define STUDENTPARSER_H

#include <QList>
#include <QString>
#include "Student.h"

class StudentParser
{
public:
    StudentParser();
    
    QList<Student> parseFile(const QString& filename);
    QList<Student> parseLine(const QString& line, int lineNumber);
    
private:
    QDate parseDate(const QString& dateStr, bool& ok);
    bool validateStudent(const Student& student);
    
    QList<Student> parseLineSimple(const QString& line, int lineNumber);
};

#endif // STUDENTPARSER_H
