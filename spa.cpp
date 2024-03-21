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
    int studentsCount = this->students.count();
    for (int i = 0; i < chromosomes.count(); i++)
    {
        ret.append("C");
        ret.append(QString::number(i+1));
        ret.append(" (fitness ");
        ret.append(QString::number(fitness(chromosomes[i])));
        ret.append("): ");
        for (int j = 0; j < studentsCount; j++)
        {
            ret.append(QString::number(chromosomes[i][j]+1));
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

    // Select two chromosomes for the binary tournament to decide the mother.
    int a = dist(gen);
    int b;
    int used;
    do
    {
        b = dist(gen);
    } while (b == a);

    // Decide the winner of the binary tournament (the mother).
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

    // Now repeat this to obtain the father.
    do
    {
        a = dist(gen);
    } while (a == used); // Mother and father cannot be the same
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
    QMap<int, int> pUsageMap1;
    QMap<int, int> pUsageMap2;
    QMap<int, int> vUsageMap1;
    QMap<int, int> vUsageMap2;
    int temp;
    int origin1Proj;
    int origin2Proj;
    Supervisor* origin1Sup;
    Supervisor* origin2Sup;
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
        origin1Sup = (origin1Proj == -1 ? nullptr : projects[origin1[i]]->getSupervisor());
        origin2Sup = (origin2Proj == -1 ? nullptr : projects[origin2[i]]->getSupervisor());

        // child 1
        if (origin1Proj != -1
            && pUsageMap1[origin1Proj] < projects[origin1Proj]->getCapacity()
            && vUsageMap1[origin1Sup->getId()] < origin1Sup->getCapacity())
        {
            child1[i] = origin1Proj;
            pUsageMap1[origin1Proj] += 1;
            vUsageMap1[origin1Sup->getId()] += 1;
        }
        else
        {
            if (origin2Proj != -1
                && pUsageMap1[origin2Proj] < projects[origin2Proj]->getCapacity()
                && vUsageMap1[origin2Sup->getId()] < origin2Sup->getCapacity())
            {
                child1[i] = origin2Proj;
                pUsageMap1[origin2Proj] += 1;
                vUsageMap1[origin2Sup->getId()] += 1;
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
        if (origin2Proj != -1
            && pUsageMap2[origin2Proj] < projects[origin2Proj]->getCapacity()
            && vUsageMap2[origin2Sup->getId()] < origin2Sup->getCapacity())
        {
            child2[i] = origin2Proj;
            pUsageMap2[origin2Proj] += 1;
            vUsageMap2[origin2Sup->getId()] += 1;
        }
        else
        {
            if (origin1Proj != -1
                && pUsageMap2[origin1Proj] < projects[origin1Proj]->getCapacity()
                && vUsageMap2[origin1Sup->getId()] < origin1Sup->getCapacity())
            {
                child2[i] = origin1Proj;
                pUsageMap2[origin1Proj] += 1;
                vUsageMap2[origin1Sup->getId()] += 1;
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
        // Todo change the loop to check for feasibility to avoid infinite loops
        if (distReals(gen) < mutation_rate)
        {
            do
            {
                temp = distProjects(gen);
            } while (pUsageMap1[temp] >= projects[temp]->getCapacity() &&
                     vUsageMap1[projects[temp]->getSupervisor()->getId()] >=
                     projects[temp]->getSupervisor()->getCapacity());
            if (child1[i] != -1)
            {
                pUsageMap1[child1[i]] -= 1;
                vUsageMap1[projects[child1[i]]->getSupervisor()->getId()] -= 1;
            }
            child1[i] = temp;
            pUsageMap1[temp] += 1;
            vUsageMap1[projects[temp]->getSupervisor()->getId()] += 1;
        }
        if (distReals(gen) < mutation_rate)
        {
            do
            {
                temp = distProjects(gen);
            } while (pUsageMap2[temp] >= projects[temp]->getCapacity() &&
                     vUsageMap2[projects[temp]->getSupervisor()->getId()] >=
                         projects[temp]->getSupervisor()->getCapacity());
            if (child2[i] != -1)
            {
                pUsageMap2[child2[i]] -= 1;
                vUsageMap2[projects[child2[i]]->getSupervisor()->getId()] -= 1;
            }
            child2[i] = temp;
            pUsageMap2[temp] += 1;
            vUsageMap2[projects[temp]->getSupervisor()->getId()] += 1;
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

    students = QList<Student*>();
    supervisors = QList<Supervisor*>();
    projects = QList<Project*>();

    // students

    for (int index = 1; index <= studentCount; index++)
    {
        list = QString(data->readLine()).split(u' ', Qt::SkipEmptyParts);
        list.pop_front();
        // Now check the preferences
        PrefsMap prefs;
        foreach(QString x, list)
        {
            QStringList pref = x.split(u':', Qt::SkipEmptyParts);
            int prefProj = pref[0].toInt()-1;
            int prefRank = pref[1].toInt();
            prefs[prefProj] = prefRank;
        }
        students.push_back(new Student(index-1, prefs));
    }

    // supervisors

    for (int index = 1; index <= supervisorCount; index++)
    {
        list = QString(data->readLine()).split(u' ', Qt::SkipEmptyParts);
        QStringList v = list[0].split(u':', Qt::SkipEmptyParts);
        int capacity = v[1].toInt();
        list.pop_front();
        // Now check the preferences
        PrefsMap prefs;
        foreach(QString x, list)
        {
            QStringList pref = x.split(u':', Qt::SkipEmptyParts);
            int prefProj = pref[0].toInt()-1;
            int prefRank = pref[1].toInt();
            prefs[prefProj] = prefRank;
        }
        supervisors.push_back(new Supervisor(index-1, prefs, capacity));
    }

    // projects

    for (int index = 1; index <= projectCount; index++)
    {
        list = QString(data->readLine()).split(u' ', Qt::SkipEmptyParts);
        QStringList p = list[0].split(u':', Qt::SkipEmptyParts);
        int capacity = p[1].toInt();
        list.pop_front();
        Supervisor* linkedV = supervisors[list[0].toInt()-1];
        projects.push_back(new Project(index-1, linkedV, capacity));
    }

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

