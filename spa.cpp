#include "spa.h"
#include <random>
#include <chrono>

Supervisor::Supervisor(int id, PrefsMap prefs)
{
    this->id = id;
    this->prefs = prefs;
}

int Supervisor::getId()
{
    return this->id;
}

int Supervisor::getPref(int student)
{
    return this->prefs[student];
}

Student::Student(int id, PrefsMap prefs)
{
    this->id = id;
    this->prefs = prefs;
}

int Student::getId()
{
    return this->id;
}

int Student::getPref(int project)
{
    return this->prefs[project];
}

int Chromosome::getProject(int student)
{
    return this->matching[student];
}

void Chromosome::setProject(int student, int project)
{
    this->matching[student] = project;
}

QList<Chromosome *> &SPAInstance::getChromosomes()
{
    return this->chromosomes;
}

SPAInstance::SPAInstance(QList<Student*> studentSet, QList<Supervisor*> projectSet, QList<Supervisor*> supervisorSet, int popSize)
{
    this->students = studentSet;
    this->supervisors = supervisorSet;
    this->projects = projectSet;
    this->popSize = popSize;
    this->best = INT_MIN;
    this->worst = INT_MAX;

    for (int i = 0; i < popSize; i++)
    {
        this->chromosomes.push_back(new Chromosome(studentSet, projectSet));
    }
}

QList<int> Chromosome::getMatching()
{
    return this->matching;
}

Chromosome::Chromosome(QList<Student*> studentSet, QList<Supervisor*> projectSet)
{
    std::mt19937 gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> dist(0, projectSet.count()-1);

    this->matching.resize(studentSet.count(), -1);

    QMap<int, bool> assigned; // Project assigned?
    bool assignable;
    int proj;
    for (int i = 0; i < studentSet.count(); i++)
    {
        // Can we assign this student a project?
        assignable = false;
        for (int j = 0; j < projectSet.count(); j++)
        {
            if (assigned[j] == false && studentSet[i]->getPref(j) > 0 && projectSet[j]->getPref(i) > 0)
            {
                assignable = true;
            }
        }
        if (assignable == true) // Skip if can't assign
        {
            do
            {
                proj = dist(gen);
            } while (assigned[proj] != false || studentSet[i]->getPref(proj) <= 0 || projectSet[proj]->getPref(i) <= 0);
            this->matching[i] = proj;
            assigned[proj] = true;
        }
        else
        {
            this->matching[i] = -1;
        }
    }
}

QString SPAInstance::getState(void)
{
    QString ret = "";
    for (int i = 0; i < this->popSize; i++)
    {
        ret.append("C");
        ret.append(QString::number(i));
        ret.append(" (fitness ");
        ret.append(QString::number(this->chromosomes[i]->fitness(this->students, this->projects, this->supervisors, this->chromosomes[i]->getMatching())));
        ret.append("): ");
        ret.append(this->chromosomes[i]->getState());
        ret.append("\n");
    }
    return ret;
}

QString Chromosome::getState(void)
{
    QString ret = "";
    for (int i = 0; i < this->matching.count(); i++)
    {
        ret.append(QString::number(this->matching[i]));
        ret.append(" ");
    }
    return ret;
}

int Chromosome::fitness(QList<Student*> studentSet, QList<Supervisor*> projectSet, QList<Supervisor*> supervisorSet, QList<int> matching)
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
            s_match = studentSet[i]->getPref(matching[i]);
            if (s_match > 0)
            {
                fitness += (solution_size / pow((s_match), 2));
            }

            // Lecturer preferences
            v_match = projectSet[matching[i]]->getPref(i);
            if (v_match > 0)
            {
                fitness += (solution_size / pow((v_match), 2));
            }
        }
    }
    return fitness;
}

void SPAInstance::iterateSPA(void)
{
    std::mt19937 gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    std::uniform_int_distribution<> dist(0, this->chromosomes.count()-1);

    int studentsCount = this->students.count();
    std::uniform_int_distribution<> distStudents(0, studentsCount-1);
    int projectsCount = this->projects.count();
    std::uniform_int_distribution<> distProjects(0, projectsCount-1);
    std::uniform_real_distribution<> distReals(0.0, 1.0);

    QList<int> child1(studentsCount);
    QList<int> child2(studentsCount);
    Chromosome* mother;
    Chromosome* father;

    // SELECTION

    // Select two chromosomes in a binary tournament.
    int a = dist(gen);
    int b;
    int used;
    do
    {
        b = dist(gen);
    } while (b == a);

    if (this->chromosomes[a]->fitness(this->students, this->projects, this->supervisors, chromosomes[a]->getMatching()) >=
        this->chromosomes[b]->fitness(this->students, this->projects, this->supervisors, chromosomes[b]->getMatching()))
    {
        mother = this->chromosomes[a];
        used = a;
    }
    else
    {
        mother = this->chromosomes[b];
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
    if (this->chromosomes[a]->fitness(this->students, this->projects, this->supervisors, chromosomes[a]->getMatching()) >=
        this->chromosomes[b]->fitness(this->students, this->projects, this->supervisors, chromosomes[b]->getMatching()))
    {
        father = this->chromosomes[a];
    }
    else
    {
        father = this->chromosomes[b];
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
    Chromosome* origin1;
    Chromosome* origin2;
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

        origin1Proj = origin1->getProject(i);
        origin2Proj = origin2->getProject(i);

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
    int child1Fitness = mother->fitness(this->students, this->projects, this->supervisors, child1);
    int child2Fitness = mother->fitness(this->students, this->projects, this->supervisors, child2);
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
        fitnesses[i] = this->chromosomes[i]->fitness(this->students, this->projects, this->supervisors, this->chromosomes[i]->getMatching());
        fitnessesCopy[i] = this->chromosomes[i]->fitness(this->students, this->projects, this->supervisors, this->chromosomes[i]->getMatching());
    }
    std::sort(fitnesses.begin(), fitnesses.end());
    this->best = std::max(this->best, fitnesses.back());
    this->worst = std::min(this->worst, fitnesses.front());
    if (fitnesses[0] < child1Fitness)
    {
        int t = fitnessesCopy.indexOf(fitnesses[0]);
        for (int i = 0; i < studentsCount; i++)
        {
            this->chromosomes[t]->setProject(i, child1[i]);
        }
        if (fitnesses[1] < child2Fitness)
        {
            t = fitnessesCopy.indexOf(fitnesses[1]);
            for (int i = 0; i < studentsCount; i++)
            {
                this->chromosomes[t]->setProject(i, child2[i]);
            }
        }
    }
}

int SPAInstance::bestFitness(void)
{
    return this->best;
}

int SPAInstance::worstFitness(void)
{
    return this->worst;
}
