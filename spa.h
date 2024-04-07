#ifndef SPA_H
#define SPA_H

#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
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
    int capacity;
    PrefsMap prefs;
public:
    int getId(void);
    int getCapacity(void);
    int getPref(int student);
    Supervisor(int id, PrefsMap prefs, int capacity);
};

class Student
{
private:
    int id;
    PrefsMap prefs;
public:
    int getId(void);
    int getPref(int project);
    Student(int id, PrefsMap prefs);
};

class Project
{
private:
    int id;
    int capacity;
    Supervisor* supervisor;
public:
    int getId(void);
    int getCapacity(void);
    Supervisor* getSupervisor(void);
    Project(void);
    Project(int id, Supervisor* supervisor, int capacity);
};

class GAInstance
{
private:
    QList<QList<int>> chromosomes;
    QList<int> fitnesses;
    int best;
    int worst;
    QList<int> bestChromosome;
    double mutation_rate;
    int getSolutionSize(QList<int> matching);
public:
    QList<Student*> students;
    QList<Project*> projects;
    QList<Supervisor*> supervisors;
    QString getState(void);
    void iterateSPA(void);
    int bestFitness(void);
    int worstFitness(void);
    GAInstance(QFile* data, int);
    int fitness(QList<int> matching);
    QString getBestChromosome(void);
    void setMutationRate(double rate);
    int bestSolutionSize(void);
};

#endif // SPA_H
