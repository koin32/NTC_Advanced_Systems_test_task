#include "StudentManager.h"
#include "StudentParser.h"
#include <QDebug>
#include <QDataStream>
#include <QIODevice>

StudentManager::StudentManager()
{
}

void StudentManager::loadStudentsFromFiles(const QStringList& filenames)
{
    m_students.clear();
    StudentParser parser;
    
    for (const QString& filename : filenames) {
        QList<Student> fileStudents = parser.parseFile(filename);
        m_students.append(fileStudents);
        qDebug() << "Loaded" << fileStudents.size() << "students from" << filename;
    }
    
    mergeDuplicates();
    qDebug() << "Total unique students:" << m_students.size();
}

QList<Student> StudentManager::getUniqueStudents() const
{
    return m_students;
}

QByteArray StudentManager::serializeStudents() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_15);
    
    int count = m_students.size();
    stream << count;
    
    qDebug() << "Serializing" << count << "students";
    
    for (const Student& student : m_students) {
        stream << student.id() 
               << student.firstName() 
               << student.middleName() 
               << student.lastName() 
               << student.birthDate();
        
        qDebug() << "  -" << student.id() << student.fullName() << student.birthDate().toString("dd.MM.yyyy");
    }
    
    qDebug() << "Serialized data size:" << data.size() << "bytes";
    qDebug() << "First 100 bytes hex:" << data.left(100).toHex();
    
    return data;
}

void StudentManager::mergeDuplicates()
{
    QList<Student> uniqueStudents;
    QSet<QString> seenStudents;
    
    for (const Student& student : m_students) {
        QString key = QString("%1|%2|%3|%4")
            .arg(student.firstName())
            .arg(student.middleName())
            .arg(student.lastName())
            .arg(student.birthDate().toString("yyyy-MM-dd")); // Унифицированный формат
        
        if (!seenStudents.contains(key)) {
            seenStudents.insert(key);
            uniqueStudents.append(student);
        }
    }
    
    m_students = uniqueStudents;
}
