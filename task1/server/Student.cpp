#include "Student.h"
#include <QDebug>

Student::Student() 
    : m_id(-1)
{
}

Student::Student(int id, const QString& firstName, const QString& middleName,
                 const QString& lastName, const QDate& birthDate)
    : m_id(id)
    , m_firstName(firstName)
    , m_middleName(middleName)
    , m_lastName(lastName)
    , m_birthDate(birthDate)
{
}

bool Student::isValid() const
{
    return m_id > 0 && 
           !m_firstName.isEmpty() && 
           !m_lastName.isEmpty() && 
           m_birthDate.isValid();
}

QString Student::fullName() const
{
    if (m_middleName.isEmpty()) {
        return QString("%1 %2").arg(m_lastName, m_firstName).simplified();
    } else {
        return QString("%1 %2 %3").arg(m_lastName, m_firstName, m_middleName).simplified();
    }
}

bool Student::operator==(const Student& other) const
{
    return m_firstName == other.m_firstName &&
           m_middleName == other.m_middleName &&
           m_lastName == other.m_lastName &&
           m_birthDate == other.m_birthDate;
}
