#include "spa.h"
#include <random>
#include <chrono>
#include <QFile>

// SUPERVISOR

Supervisor::Supervisor(int id, PrefsMap prefs, int capacity)
{
    this->id = id;
    this->prefs = prefs;
    this->capacity = capacity;
}

int Supervisor::getId()
{
    return id;
}

int Supervisor::getCapacity()
{
    return capacity;
}

int Supervisor::getPref(int student)
{
    return prefs[student];
}

// STUDENT

Student::Student(int id, PrefsMap prefs)
{
    this->id = id;
    this->prefs = prefs;
}

int Student::getId()
{
    return id;
}

int Student::getPref(int project)
{
    return prefs[project];
}

// PROJECT

Project::Project(int id, Supervisor* supervisor, int capacity)
{
    this->id = id;
    this->supervisor = supervisor;
    this->capacity = capacity;
}

Supervisor* Project::getSupervisor()
{
    return supervisor;
}

int Project::getCapacity()
{
    return capacity;
}

// GAINSTANCE

QString GAInstance::getState(void)
{
    QString ret = "";
    for (int i = 0; i < chromosomes.count(); i++)
    {
        ret.append("C");
        ret.append(QString::number(i));
        ret.append(" (fitness ");
        ret.append(QString::number(fitness(chromosomes[i])));
        ret.append("): ");
        for (int j = 0; i < chromosomes[i].count(); i++)
        {
            ret.append(QString::number(chromosomes[i][j]));
            ret.append(" ");
        }
        ret.append("\n");
    }
    return ret;
}

int GAInstance::fitness(QList<int> matching)
{
    int fitness = 0;
    int solution_size = 0;
    int s_match, v_match;
    // Loop through solution
    for (int i = 0; i < matching.count(); i++)
    {
        if (matching[i] != -1)
            solution_size++;
    }

    solution_size = pow(solution_size, 1);

    for (int i = 0; i < matching.count(); i++)
    {
        if (matching[i] != -1)
        {
            // Student preferences
            s_match = students[i]->getPref(matching[i]);
            if (s_match > 0)
            {
                fitness += (solution_size / pow((s_match), 2));
            }

            // Lecturer preferences
            v_match = projects[matching[i]]->getSupervisor()->getPref(i);
            if (v_match > 0)
            {
                fitness += (solution_size / pow((v_match), 2));
            }
        }
    }
    return fitness;
}

// GAINSTANCE

void GAInstance::iterateSPA(void)
{
    std::mt19937 gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    std::uniform_int_distribution<> dist(0, chromosomes.count()-1);

    int studentsCount = students.count();
    std::uniform_int_distribution<> distStudents(0, studentsCount-1);
    int projectsCount = projects.count();
    std::uniform_int_distribution<> distProjects(0, projectsCount-1);
    std::uniform_real_distribution<> distReals(0.0, 1.0);

    QList<int> child1(studentsCount);
    QList<int> child2(studentsCount);
    QList<int> mother;
    QList<int> father;

    // SELECTION

    // Select two chromosomes in a binary tournament.
    int a = dist(gen);
    int b;
    int used;
    do
    {
        b = dist(gen);
    } while (b == a);

    if (fitness(chromosomes[a]) >= fitness(chromosomes[b]))
    {
        mother = chromosomes[a];
        used = a;
    }
    else
    {
        mother = chromosomes[b];
        used = b;
    }

    a = used;
    while (a == used)
    {
        a = dist(gen);
    }
    b = a;
    while (b == a || b == used)
    {
        b = dist(gen);
    }
    if (fitness(chromosomes[a]) >= fitness(chromosomes[b]))
    {
        father = chromosomes[a];
    }
    else
    {
        father = chromosomes[b];
    }

    // CROSSOVER
    int crosspoint1 = distStudents(gen);
    int crosspoint2 = crosspoint1;
    while(crosspoint2 == crosspoint1)
    {
        crosspoint2 = distStudents(gen);
    }
    if (crosspoint1 > crosspoint2) std::swap(crosspoint1, crosspoint2);
    QMap<int, bool> usageMap1;
    QMap<int, bool> usageMap2;
    int temp;
    int origin1Proj;
    int origin2Proj;
    QList<int> origin1;
    QList<int> origin2;
    for (int i = 0; i < studentsCount; i++)
    {
        if (i <= crosspoint1 || i >= crosspoint2)
        {
            origin1 = mother;
            origin2 = father;
        }
        else
        {
            origin1 = father;
            origin2 = mother;
        }

        origin1Proj = origin1[i];
        origin2Proj = origin2[i];

        // child 1
        if (usageMap1[origin1Proj] == false)
        {
            child1[i] = origin1Proj;
            usageMap1[origin1Proj] = true;
        }
        else
        {
            if (usageMap1[origin2Proj] == false)
            {
                child1[i] = origin2Proj;
                usageMap1[origin2Proj] = true;
            }
            else // Just generate a new project that hasn't already been used
            {
                /*temp = origin1Proj;
                while (usageMap1[temp] == true)
                {
                    temp = distProjects(gen);
                }
                child1[i] = temp;
                usageMap1[temp] = true;*/
                child1[i] = -1;
            }
        }

        // child 2
        if (usageMap2[origin2Proj] == false)
        {
            child2[i] = origin2Proj;
            usageMap2[origin2Proj] = true;
        }
        else
        {
            if (usageMap2[origin1Proj] == false)
            {
                child2[i] = origin1Proj;
                usageMap2[origin1Proj] = true;
            }
            else // Just generate a new project that hasn't already been used
            {
                /*temp = origin2Proj;
                while (usageMap2[temp] == true)
                {
                    temp = distProjects(gen);
                }
                child2[i] = temp;
                usageMap2[temp] = true;*/
                child2[i] = -1;
            }
        }
    }

    // MUTATION
    double mutation_rate = 1.0 / (this->chromosomes.count() * std::sqrt(this->students.count()));
    for (int i = 0; i < studentsCount; i++)
    {
        if (distReals(gen) < mutation_rate)
        {
            do
            {
                temp = distProjects(gen);
            } while (usageMap1[temp] == true);
            usageMap1[child1[i]] = false;
            child1[i] = temp;
            usageMap1[temp] = true;
        }
        if (distReals(gen) < mutation_rate)
        {
            do
            {
                temp = distProjects(gen);
            } while (usageMap2[temp] == true);
            usageMap2[child2[i]] = false;
            child2[i] = temp;
            usageMap2[temp] = true;
        }
    }

    // REPLACEMENT
    int child1Fitness = fitness(child1);
    int child2Fitness = fitness(child2);
    if (child2Fitness > child1Fitness)
    {
        std::swap(child1, child2);
        std::swap(child1Fitness, child2Fitness);
    }
    QList<int> fitnesses(this->chromosomes.count());
    this->fitnesses = fitnesses;
    QList<int> fitnessesCopy(this->chromosomes.count());
    for (int i = 0; i < this->chromosomes.count(); i++)
    {
        fitnesses[i] = fitness(chromosomes[i]);
        fitnessesCopy[i] = fitnesses[i];
    }
    std::sort(fitnesses.begin(), fitnesses.end());
    this->best = std::max(this->best, fitnesses.back());
    this->worst = std::min(this->worst, fitnesses.front());
    if (fitnesses[0] < child1Fitness)
    {
        int t = fitnessesCopy.indexOf(fitnesses[0]);
        for (int i = 0; i < studentsCount; i++)
        {
            this->chromosomes[t][i] = child1[i];
        }
        if (fitnesses[1] < child2Fitness)
        {
            t = fitnessesCopy.indexOf(fitnesses[1]);
            for (int i = 0; i < studentsCount; i++)
            {
                this->chromosomes[t][i] = child2[i];
            }
        }
    }
}

int GAInstance::bestFitness(void)
{
    return this->best;
}

int GAInstance::worstFitness(void)
{
    return this->worst;
}

GAInstance::GAInstance(QFile* data, int size)
{
    // First initialise dataset-independent values

    best = INT_MIN;
    worst = INT_MAX;

    // Parse the input file to create student/supervisor/project sets

    QStringList list = QString(data->readLine()).split(u' ', Qt::SkipEmptyParts);

    int studentCount = list[0].toInt();
    int supervisorCount = list[1].toInt();
    int projectCount = list[2].toInt();

    students = QList<Student*>(studentCount);
    supervisors = QList<Supervisor*>(supervisorCount);
    projects = QList<Project*>(projectCount);

    // Done making the student/supervisor/project sets, now initialise the chromosomes

    std::mt19937 gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> dist(0, projects.count()-1);

    for (int i = 0; i < size; i++)
    {
        QList<int> chromosome = QList<int>(students.count());

        QMap<int, bool> assigned; // Project assigned?
        bool assignable;
        int proj;
        for (int i = 0; i < students.count(); i++)
        {
            // Can we assign this student a project?
            assignable = false;
            for (int j = 0; j < projects.count(); j++)
            {
                if (assigned[j] == false && students[i]->getPref(j) > 0 && projects[j]->getSupervisor()->getPref(i) > 0)
                {
                    assignable = true;
                }
            }
            if (assignable == true) // Skip if can't assign
            {
                do
                {
                    proj = dist(gen);
                } while (assigned[proj] != false || students[i]->getPref(proj) <= 0 || projects[proj]->getSupervisor()->getPref(i) <= 0);
                chromosome[i] = proj;
                assigned[proj] = true;
            }
            else
            {
                chromosome[i] = -1;
            }
        }
        chromosomes.push_back(chromosome);
    }
}

