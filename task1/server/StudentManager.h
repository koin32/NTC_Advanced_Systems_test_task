#ifndef STUDENTMANAGER_H
#define STUDENTMANAGER_H

#include <QList>
#include <QSet>
#include "Student.h"

class StudentManager
{
public:
    StudentManager();
    
    void loadStudentsFromFiles(const QStringList& filenames);
    QList<Student> getUniqueStudents() const;
    QByteArray serializeStudents() const;
    
private:
    QList<Student> m_students;
    
    void mergeDuplicates();
};

#endif // STUDENTMANAGER_H
