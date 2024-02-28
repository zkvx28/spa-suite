#include "spa.h"

Supervisor::Supervisor()
{
    this->id = 0;
    this->prefs = {0};
}

Supervisor::Supervisor(int id, QList<int> prefs)
{
    this->id = id;
    this->prefs = prefs;
}

int Supervisor::getId()
{
    return this->id;
}

QList<int> Supervisor::getPrefs()
{
    return this->prefs;
}

Student::Student()
{
    this->id = 0;
    this->prefs = {0};
}

Student::Student(int id, QList<int> prefs)
{
    this->id = id;
    this->prefs = prefs;
}

int Student::getId()
{
    return this->id;
}

QList<int> Student::getPrefs()
{
    return this->prefs;
}
