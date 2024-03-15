#ifndef SPA_H
#define SPA_H

#include <QString>
#include <QList>
#include <QMap>
#include <random>

class PrefsMap : public QMap<int, int> {
public:
    int value(int key) const {
        return contains(key) ? QMap<int, int>::value(key) : -1;
    }
};

class Supervisor
{
private:
    int id;
    PrefsMap prefs;
public:
    int getId(void);
    int getPref(int student);
    Supervisor(int id, PrefsMap prefs);
};

class Student
{
private:
    int id;
    PrefsMap prefs;
public:
    int getId(void);
    int getPref(int project);
    QList<int> getGreaterPrefs(int project);
    Student(int id, PrefsMap prefs);
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

class Chromosome
{
private:
    QList<int> matching;
public:
    QList<int> getMatching(void);
    int getProject(int student);
    void setProject(int student, int project);
    int fitness(QList<Student*> studentSet, QList<Supervisor*> projectSet, QList<Supervisor*> supervisorSet, QList<int> matching);
    QString getState(void);
    bool stable(QList<Student*> studentSet, QList<Supervisor*> projectSet);
    Chromosome(QList<Student*> studentSet, QList<Supervisor*> projectSet);
};

class SPAInstance
{
private:
    QList<Student*> students;
    QList<Supervisor*> projects;
    QList<Supervisor*> supervisors;
    QList<Chromosome*> chromosomes;
    QList<int> fitnesses;
    int best;
    int worst;
    int popSize;
public:
    QList<Chromosome*>& getChromosomes(void);
    QString getState(void);
    SPAInstance(QList<Student*> studentSet, QList<Supervisor*> projectSet, QList<Supervisor*> supervisorSet, int popSize);
    void iterateSPA(void);
    int bestFitness(void);
    int worstFitness(void);
};

#endif // SPA_H
