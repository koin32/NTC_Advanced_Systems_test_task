#ifndef STUDENT_H
#define STUDENT_H

#include <QString>
#include <QDate>
#include <QMetaType>

class Student
{
public:
    Student();
    Student(int id, const QString& firstName, const QString& middleName, 
            const QString& lastName, const QDate& birthDate);
    
    bool isValid() const;
    QString fullName() const;
    
    int id() const { return m_id; }
    QString firstName() const { return m_firstName; }
    QString middleName() const { return m_middleName; }
    QString lastName() const { return m_lastName; }
    QDate birthDate() const { return m_birthDate; }
    
    bool operator<(const Student& other) const;
    
private:
    int m_id;
    QString m_firstName;
    QString m_middleName;
    QString m_lastName;
    QDate m_birthDate;
};

Q_DECLARE_METATYPE(Student)

#endif // STUDENT_H
