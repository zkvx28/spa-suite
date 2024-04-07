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
    ret.append("Current best fitness: ");
    ret.append(QString::number(best));
    ret.append("\n=CHROMOSOMES==========\n");
    int studentsCount = this->students.count();
    for (int i = 0; i < chromosomes.count(); i++)
    {
        ret.append("C");
        ret.append(QString::number(i+1));
        ret.append(" (fitness ");
        ret.append(QString::number(fitnesses[i]));
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

    // Penalty initialisation
    QList<int> penaltiesProjects(projects.count());
    QList<int> penaltiesSupervisors(supervisors.count());
    for (int i = 0; i < projects.count(); i++)
    {
        penaltiesProjects[i] = projects[i]->getCapacity();
    }
    for (int i = 0; i < supervisors.count(); i++)
    {
        penaltiesSupervisors[i] = supervisors[i]->getCapacity();
    }
    for (int i = 0; i < matching.count(); i++)
    {
        // Project/supervisor penalties
        if (matching[i] != -1)
        {
            penaltiesProjects[matching[i]]--;
            penaltiesSupervisors[projects[matching[i]]->getSupervisor()->getId()]--;
        }
    }

    // Loop through solution, don't increase solution size if capacity reached for project/supervisor
    for (int i = 0; i < matching.count(); i++)
    {
        if (matching[i] != -1 &&
            penaltiesProjects[matching[i]] >= 0 &&
            penaltiesSupervisors[projects[matching[i]]->getSupervisor()->getId()] >= 0 &&
            students[i]->getPref(matching[i]) > 0 &&
            projects[matching[i]]->getSupervisor()->getPref(i) > 0)
        {
            solution_size++;
        }
    }

    for (int i = 0; i < matching.count(); i++)
    {
        if (matching[i] != -1)
        {
            fitness += students.count();

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

    int penalty = std::accumulate(std::begin(penaltiesProjects), std::end(penaltiesProjects), 0, [](int total, int value) { return value < 0 ? total + value : total; });
    penalty += std::accumulate(std::begin(penaltiesSupervisors), std::end(penaltiesSupervisors), 0, [](int total, int value) { return value < 0 ? total + value : total; });
    if (penalty != 0)
        fitness = 0;
    fitness += students.count() * penalty;

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
        if (origin1Proj == -1)
        {
            child1[i] = -1;
        }
        else if (pUsageMap1[origin1Proj] < projects[origin1Proj]->getCapacity()
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
            else
            {
                child1[i] = -1;
            }
        }

        // child 2
        if (origin2Proj == -1)
        {
            child2[i] = -1;
        }
        else if (pUsageMap2[origin2Proj] < projects[origin2Proj]->getCapacity()
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
            else
            {
                child2[i] = -1;
            }
        }
    }

    // MUTATION
    double rand_result;
    for (int i = 0; i < studentsCount; i++)
    {
        // Child 1
        rand_result = distReals(gen);
        if (rand_result < (mutation_rate / 2))
        {
            // First obtain a list of projects that are eligible
            QList<int> projsToChoose(0);
            QList<int> backupProjs(0);
            for (int j = 0; j < projects.count(); j++)
            {
                if (pUsageMap1[j] < projects[j]->getCapacity() &&
                    vUsageMap1[projects[j]->getSupervisor()->getId()] < projects[j]->getSupervisor()->getCapacity())
                {
                    if (students[i]->getPref(j) > 0 && projects[j]->getSupervisor()->getPref(i) > 0)
                    {
                        projsToChoose.push_back(j);
                    }
                    backupProjs.push_back(j);
                }
            }

            if (projsToChoose.count() > 0)
            {
                std::uniform_int_distribution<> distProjsToChoose(0, projsToChoose.count()-1);

                temp = projsToChoose[distProjsToChoose(gen)];

                if (child1[i] != -1)
                {
                    pUsageMap2[child1[i]] -= 1;
                    vUsageMap2[projects[child1[i]]->getSupervisor()->getId()] -= 1;
                }
                child1[i] = temp;
                pUsageMap1[temp] += 1;
                vUsageMap1[projects[temp]->getSupervisor()->getId()] += 1;
            }
            else if (backupProjs.count() > 0) // Assign anything as long as it doesn't break capacity constraints
            {
                std::uniform_int_distribution<> distBackupProjs(0, backupProjs.count()-1);

                temp = backupProjs[distBackupProjs(gen)];

                if (child1[i] != -1)
                {
                    pUsageMap1[child1[i]] -= 1;
                    vUsageMap1[projects[child1[i]]->getSupervisor()->getId()] -= 1;
                }
                child1[i] = temp;
                pUsageMap1[temp] += 1;
                vUsageMap1[projects[temp]->getSupervisor()->getId()] += 1;
            } // Else cannot mutate this gene
        }
        else if (rand_result < mutation_rate)
        {
            if (child1[i] != -1)
            {
                pUsageMap1[child1[i]] -= 1;
                vUsageMap1[projects[child1[i]]->getSupervisor()->getId()] -= 1;
            }
            child1[i] = -1;
        }
        // Child 2
        rand_result = distReals(gen);
        if (rand_result < (mutation_rate / 2))
        {
            // First obtain a list of projects that are eligible
            QList<int> projsToChoose(0);
            QList<int> backupProjs(0);
            for (int j = 0; j < projects.count(); j++)
            {
                if (pUsageMap2[j] < projects[j]->getCapacity() &&
                    vUsageMap2[projects[j]->getSupervisor()->getId()] < projects[j]->getSupervisor()->getCapacity())
                {
                    if (students[i]->getPref(j) > 0 && projects[j]->getSupervisor()->getPref(i) > 0)
                    {
                        projsToChoose.push_back(j);
                    }
                    backupProjs.push_back(j);
                }
            }

            if (projsToChoose.count() > 0)
            {
                std::uniform_int_distribution<> distProjsToChoose(0, projsToChoose.count()-1);

                temp = projsToChoose[distProjsToChoose(gen)];

                if (child2[i] != -1)
                {
                    pUsageMap2[child2[i]] -= 1;
                    vUsageMap2[projects[child2[i]]->getSupervisor()->getId()] -= 1;
                }
                child2[i] = temp;
                pUsageMap2[temp] += 1;
                vUsageMap2[projects[temp]->getSupervisor()->getId()] += 1;
            }
            else if (backupProjs.count() > 0) // Assign anything as long as it doesn't break capacity constraints
            {
                std::uniform_int_distribution<> distBackupProjs(0, backupProjs.count()-1);

                temp = backupProjs[distBackupProjs(gen)];

                if (child2[i] != -1)
                {
                    pUsageMap2[child2[i]] -= 1;
                    vUsageMap2[projects[child2[i]]->getSupervisor()->getId()] -= 1;
                }
                child2[i] = temp;
                pUsageMap2[temp] += 1;
                vUsageMap2[projects[temp]->getSupervisor()->getId()] += 1;
            } // Else cannot mutate this gene
        }
        else if (rand_result < mutation_rate)
        {
            if (child2[i] != -1)
            {
                pUsageMap2[child2[i]] -= 1;
                vUsageMap2[projects[child2[i]]->getSupervisor()->getId()] -= 1;
            }
            child2[i] = -1;
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
    QList<int> fitnessesCopy(this->chromosomes.count());
    for (int i = 0; i < this->chromosomes.count(); i++)
    {
        fitnesses[i] = this->fitnesses[i];
        fitnessesCopy[i] = fitnesses[i];
    }
    std::sort(fitnesses.begin(), fitnesses.end());
    if (fitnesses.back() > this->best)
    {
        int t = fitnessesCopy.indexOf(fitnesses.back());
        for (int i = 0; i < studentsCount; i++)
        {
            this->bestChromosome[i] = chromosomes[t][i];
        }
    }
    this->best = std::max(this->best, fitnesses.back());
    this->worst = std::min(this->worst, fitnesses.front());
    if (fitnesses[0] < child1Fitness)
    {
        int t = fitnessesCopy.indexOf(fitnesses[0]);
        for (int i = 0; i < studentsCount; i++)
        {
            this->chromosomes[t][i] = child1[i];
        }
        this->fitnesses[t] = child1Fitness; // Replace with child 1's fitness
        if (fitnesses[1] < child2Fitness)
        {
            t = fitnessesCopy.indexOf(fitnesses[1]);
            for (int i = 0; i < studentsCount; i++)
            {
                this->chromosomes[t][i] = child2[i];
            }
            this->fitnesses[t] = child2Fitness; // Replace with child 2's fitness
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
    bestChromosome = QList<int>(studentCount);
    fitnesses = QList<int>();

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
    std::uniform_real_distribution<> distReals(0.0, 1.0);

    for (int i = 0; i < size; i++)
    {
        QList<int> chromosome = QList<int>(students.count());

        QMap<int, int> pCapacity; // Project capacity reached?
        QMap<int, int> vCapacity; // Supervisor capacity reached?
        bool assignable;
        int proj;
        for (int j = 0; j < students.count(); j++)
        {
            // Small probability to assign nothing
            if (distReals(gen) < 0.2)
            {
                chromosome[j] = -1;
                continue;
            }
            // Can we assign this student a project?
            assignable = false;
            for (int k = 0; k < projects.count(); k++)
            {
                if (pCapacity[k] < projects[k]->getCapacity() &&
                    students[j]->getPref(k) > 0 &&
                    projects[k]->getSupervisor()->getPref(j) > 0)
                {
                    assignable = true;
                }
            }
            if (assignable == true) // Skip if can't assign
            {
                do
                {
                    proj = dist(gen);
                } while (pCapacity[proj] >= projects[proj]->getCapacity() ||
                         students[j]->getPref(proj) <= 0 ||
                         projects[proj]->getSupervisor()->getPref(j) <= 0);
                chromosome[j] = proj;
                pCapacity[proj] += 1;
            }
            else
            {
                chromosome[j] = -1;
            }
        }
        chromosomes.push_back(chromosome);
        fitnesses.push_back(fitness(chromosome));
    }
}

QString GAInstance::getBestChromosome(void)
{
    QString ret = "";
    for (int i = 0; i < students.count(); i++)
    {
        ret += QString::number(bestChromosome[i]+1);
        ret += " ";
    }
    return ret;
}

void GAInstance::setMutationRate(double rate)
{
    mutation_rate = rate;
}

int GAInstance::getSolutionSize(QList<int> matching)
{
    int solution_size = 0;

    for (int i = 0; i < matching.count(); i++)
    {
        if (matching[i] != -1)
            solution_size++;
    }

    return solution_size;
}

int GAInstance::bestSolutionSize(void)
{
    return getSolutionSize(bestChromosome);
}
