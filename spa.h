#ifndef SPA_H
#define SPA_H

#include <QString>
#include <QList>

class Supervisor
{
private:
    int id;
    QList<int> prefs;
public:
    int getId(void);
    QList<int> getPrefs(void);
    Supervisor(void);
    Supervisor(int id, QList<int> prefs);

};

class Student
{
private:
    int id;
    QList<int> prefs;
public:
    int getId(void);
    QList<int> getPrefs(void);
    Student(void);
    Student(int id, QList<int> prefs);
};

class Project
{
private:
    int id;
    Supervisor* supervisor;
public:
    int getId(void);
    Supervisor* getSupervisor(void);
    Project(void);
    Project(int id, Supervisor* supervisor);
};

#endif // SPA_H
