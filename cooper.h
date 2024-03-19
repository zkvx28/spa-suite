#ifndef COOPER_H
#define COOPER_H

#include <QList>
#include <QString>
#include <QFile>

// For implementation of Frances Cooper's 3/2 approximation algorithm
// Translated from Java into C++ referencing https://github.com/fmcooper/stable-SPA

class FCLecturer;
class FCStudent;
class FCModel;

// Project.java

class FCProject
{
public:
    int pNum;
    int pInd;
    int upperQ;
    int numAssigned;
    FCLecturer* lec;
    QList<FCStudent*> students;
    FCProject(int i, int upperQ);
    bool precarious(void);
    FCStudent* getPrecarious(void);
    void addStudent(FCStudent* s);
    void removeStudent(FCStudent* s);
    void attachLec(FCLecturer* l);
    bool isFullyAvailable(void);
    // void print(void);
};

// Student.java

class FCStudent
{
private:
    QList<FCProject*> prefList;
    QList<int> ranks;
    QList<FCProject*> tempPrefList;
    QList<int> tempRanks;
    int currentFAindex = -1;
    int assignedInPhase;
public:
    int sInd;
    int sNum;
    FCProject* proj;
    int phase;
    FCStudent(int num, QList<FCProject*> prefList, QList<int> ranks);
    bool isPrecarious(void);
    int getRank(FCProject* p);
    int status(void);
    bool isAvailable(void);
    bool isProvisional(void);
    bool isConfirmed(void);
    void assigned(FCProject* p);
    void unassigned(void);
    QList<FCProject*> getPreferredProjs(void);
    void removePref(FCProject* p);
    int getFavourite(QList<FCProject*> projects);
    // void print(void);
};

// Lecturer.java

class FCLecturer
{
private:
    QList<int> ranks;
public:
    int lInd;
    int lNum;
    int numAssigned;
    QList<FCStudent*> prefList;
    QList<FCProject*> projects;
    int upperQ;
    FCLecturer(int num, QList<FCStudent*> prefList, QList<int>, int upperQ);
    void attachProj(FCProject* p);
    FCStudent* metaPrefers(FCStudent* s);
    bool wobbly(FCStudent* s);
    bool precarious(void);
    FCStudent* metaPrefers(FCStudent* s, FCProject* p);
    int getRank(FCStudent* s);
    FCStudent* getPrecarious(void);
    // void print(void);
};

// Approx.java

class FCApprox
{
private:
    FCModel* model;
    QList<FCStudent*> students;
    QList<FCProject*> projects;
    QList<FCLecturer*> lecturers;
    bool trace;
public:
    FCApprox(FCModel* model);
    void run(void);
};

// Model.java
class FCModel
{
public:
    int numStudents;
    int numProjects;
    int numLecturers;
    QList<int> projectLowerQuotas;
    QList<int> projUpperQuotas;
    QList<int> projLecturers;
    QList<int> lecturerLowerQuotas;
    QList<int> lecturerUpperQuotas;
    QList<QList<int>> studentPrefArray;
    QList<QList<int>> studentPrefRankArray;
    QList<QList<int>> lecturerPrefArray;
    QList<QList<int>> lecturerPrefRankArray;
    QList<int> lecturerTargets;
    QList<QList<int>> lecturersProjs;
    QList<QList<int>> projectAssignments;
    QList<int> studentAssignments;
    QList<int> numProjectAssignments;
    QList<int> numLecturerAssignments;
    bool feasible;
    QString infoString;
    int maxPrefLength;
    QString timeAndDate;

    FCModel(QList<QList<int>> studentPrefArray, QList<QList<int>> studentPrefRankArray, QList<QList<int>> lecturerPrefArray,
          QList<QList<int>> lecturerPrefRankArray, QList<int> projectLowerQuotas, QList<int> projUpperQuotas,
          QList<int> projLecturers, QList<int> lecturerLowerQuotas, QList<int> lecturerUpperQuotas, QList<int> lecturerTargets);
    QString getNoMatchingResult(void);
    QString getRawResults(void);
    QString getStringAssignments(void);
    void setAssignmentInfo(void);
    QString getAllResults(void);
    QString getBriefStatsFeasible(void);
    QString getBriefStatsInfeasible(void);
    int calcMatchedCost(void);
    int calcSumSqRanks(void);
    int calcSumSqDiffsAndRanks(void);
    QList<int> calcProfile(void);
    int getRank(int studentInd, int projectInd);
    int calcMatchedDegree(void);
    QString getMatchingProfile(void);
    void setInfoString(QString s);
    void printStdoutResults(void);
    void printModel(void);
    void print(QList<QList<int>> intArray, QString message);
    void print(QList<int> intArray, QString message);
private:
    int sumSqAbsLecDiffs(void);
    int sumAbsLecDiffs(void);
    QList<int> getAbsDiffs(void);
    int calcMaxAbsDiffLec(void);
    int getMatchingSize(void);
};

// Util_FileIO.java

class FCUtil_FileIO
{
private:
    FCUtil_FileIO(void);
public:
    static FCModel* readFile(QFile* file);
    QList<int> inputResult(QFile file, int numNums);
    QList<int> inputRawResult(QFile file);
    static QList<QList<int>> getPrefsAndRanks(QString stringPrefs, int startingPosition);
};

// Tester.java

class FCTester
{
private:
    FCModel* model;
public:
    //void helpAndExit(void);
    void main(QString args);
    bool checkUpperLower(void);
    bool checkStable(void);
    bool testBlockingPair(int stInd, int projAssignment, int projAssRank, int lecAssignment, int projloc, int currentRank, QList<int> matching);
    int numAssignedToProj(int p, QList<int> matching);
    int numAssignedToLec(int l, QList<int> matching);
    int getWorstRankForLec(int lec, QList<int> matching);
    int getWorstRankInPForLec(int lec, int p, QList<int> matching);
    int getRankInLecPref(int stInd, int lec, QList<int> matching);
};

#endif // COOPER_H
