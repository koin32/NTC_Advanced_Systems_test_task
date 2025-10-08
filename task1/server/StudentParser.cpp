#include "StudentParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>

StudentParser::StudentParser()
{
}

QList<Student> StudentParser::parseFile(const QString& filename)
{
    QList<Student> students;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filename;
        return students;
    }
    
    QTextStream in(&file);
    int lineNumber = 0;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;
        
        if (line.isEmpty() || line.startsWith("--")) {
            continue;
        }
        
        QList<Student> lineStudents = parseLineSimple(line, lineNumber);
        students.append(lineStudents);
    }
    
    file.close();
    return students;
}

QList<Student> StudentParser::parseLineSimple(const QString& line, int lineNumber)
{
    QList<Student> students;
    
    QStringList parts = line.split(" ", Qt::SkipEmptyParts);
    
    qDebug() << "Parsing line:" << line;
    qDebug() << "Parts count:" << parts.size();
    
    if (parts.size() >= 4) {
        bool idOk = false;
        int id = parts[0].toInt(&idOk);
        
        if (!idOk) {
            qWarning() << "Invalid ID at line" << lineNumber << ":" << parts[0];
            return students;
        }
        
        QString firstName, middleName, lastName;
        QString dateStr;
        
        if (parts.size() == 4) {
            lastName = parts[1];
            firstName = parts[2];
            dateStr = parts[3];
        } else if (parts.size() >= 5) {
            lastName = parts[1];
            firstName = parts[2];
            
            if (parts[3].contains('.') || parts[3].length() == 10) {
                dateStr = parts[3];
            } else {
                middleName = parts[3];
                dateStr = parts[4];
            }
        }
        
        bool dateOk = false;
        QDate birthDate = parseDate(dateStr, dateOk);
        
        if (dateOk) {
            Student student(id, firstName, middleName, lastName, birthDate);
            if (validateStudent(student)) {
                students.append(student);
                qDebug() << "Successfully parsed:" << student.fullName();
            } else {
                qWarning() << "Invalid student data at line" << lineNumber;
            }
        } else {
            qWarning() << "Invalid date at line" << lineNumber << ":" << dateStr;
        }
    } else {
        qWarning() << "Not enough parts at line" << lineNumber << ":" << line;
    }
    
    return students;
}

QList<Student> StudentParser::parseLine(const QString& line, int lineNumber)
{
    return parseLineSimple(line, lineNumber);
}

QDate StudentParser::parseDate(const QString& dateStr, bool& ok)
{
    QStringList dateParts = dateStr.split(".");
    
    if (dateParts.size() == 3) {
        int day = dateParts[0].toInt();
        int month = dateParts[1].toInt();
        int year = dateParts[2].toInt();
        
        QDate date(year, month, day);
        if (date.isValid()) {
            ok = true;
            return date;
        }
    }
    
    ok = false;
    return QDate();
}

bool StudentParser::validateStudent(const Student& student)
{
    return student.isValid();
}
