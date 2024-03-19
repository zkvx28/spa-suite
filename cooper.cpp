#include "cooper.h"

// For implementation of Frances Cooper's 3/2 approximation algorithm
// Translated from Java into C++ referencing https://github.com/fmcooper/stable-SPA

// Student.java

FCStudent::FCStudent(int num, QList<FCProject*> prefList, QList<int> ranks)
{
    assignedInPhase = 0;
    proj = nullptr;
    phase = 1;
    sInd = num;
    sNum = sInd + 1;

    this->prefList = prefList;
    this->ranks = ranks;
    tempPrefList = QList<FCProject*>();
    tempRanks = QList<int>();

    for (int i = 0; i < prefList.length(); i++)
    {
        tempPrefList.push_back(prefList[i]);
        tempRanks.push_back(ranks[i]);
    }
}

bool FCStudent::isPrecarious(void)
{
    int rank = getRank(proj);

    QList<FCProject*> sameRankPs = QList<FCProject*>();
    for (int i = 0; i < ranks.length(); i++)
    {
        if (ranks[i] == rank)
        {
            sameRankPs.push_back(prefList[i]);
        }
    }

    foreach (FCProject* p, sameRankPs) {
        if (p != proj){
            if (p->isFullyAvailable()) {
                return true;
            }
        }
    }

    return false;
}

int FCStudent::getRank(FCProject* p) {
    for (int i = 0; i < prefList.length(); i++)
    {
        if (prefList[i] == p)
        {
            return ranks[i];
        }
    }
    return -1;
}

int FCStudent::status(void)
{
    if (proj == nullptr) {
        return 1;
    }
    else if (assignedInPhase == 1) {
        return 2;
    }
    else {
        return 3;
    }
}

bool FCStudent::isAvailable(void)
{
    if (status() == 2) {
        return true;
    }
    return false;
}

bool FCStudent::isConfirmed(void)
{
    if (status() == 3) {
        return true;
    }
    return false;
}

void FCStudent::assigned(FCProject* p)
{
    proj = p;
    assignedInPhase = phase;
}

void FCStudent::unassigned(void)
{
    proj = nullptr;
}

QList<FCProject*> FCStudent::getPreferredProjs()
{
    if (proj == nullptr)
    {
        QList<FCProject*> ret;
        return ret;
    }
    else
    {
        int rank = getRank(proj);
        QList<FCProject*> betterProjs = QList<FCProject*>();
        for (int i = 0; i < prefList.length(); i++) {
            if (ranks[i] < rank) {
                betterProjs.push_back(prefList[i]);
            }
        }
        return betterProjs;
    }
}

void FCStudent::removePref(FCProject* p)
{
    int index = tempPrefList.indexOf(p);
    tempPrefList.remove(index);
    tempRanks.remove(index);
    if(tempPrefList.size() == 0)
    {
        if (phase == 1) {
            phase = 2;
            for (int i = 0; i < prefList.length(); i++) {
                tempPrefList.push_back(prefList[i]);
                tempRanks.push_back(ranks[i]);
            }
        }
        else {
            phase = 3;
        }
    }
}

int FCStudent::getFavourite(QList<FCProject*> projects)
{
    QList<int> favouriteProjs = QList<int>();
    int currentRank = tempRanks.at(0);

    int r = currentRank;
    int index = 0;

    while (r == currentRank && index < tempPrefList.size()) {

        int projInd = tempPrefList.at(index)->pInd;

        if (projects[projInd]->isFullyAvailable()) {
            currentFAindex = index;
            return projInd;
        }
        index++;

        if (index < tempPrefList.size()) {
            r = tempRanks.at(index);
        }
    }
    currentFAindex = 0;
    return tempPrefList.at(0)->pInd;
}

// Lecturer.java

FCLecturer::FCLecturer(int num, QList<FCStudent*> prefList, QList<int> ranks, int upperQ)
{
    numAssigned = 0;
    projects = QList<FCProject*>();
    lInd = num;
    lNum = lInd + 1;
    this->upperQ = upperQ;

    this->prefList = prefList;
    this->ranks = ranks;
}

void FCLecturer::attachProj(FCProject* p)
{
    projects.push_back(p);
}

FCStudent* FCLecturer::metaPrefers(FCStudent* s)
{
    return metaPrefers(s, nullptr);
}

bool FCLecturer::wobbly(FCStudent* s)
{
    QList<FCStudent*> assignedStudents = QList<FCStudent*>();
    for (int i = 0; i < projects.size(); i++)
    {
        for (int j = 0; j < projects.at(i)->students.size(); j++)
        {
            assignedStudents.push_back(projects.at(i)->students.at(j));
        }
    }

    int rankOrig = getRank(s);

    foreach (FCStudent* testerSt, assignedStudents) {
        if (rankOrig == getRank(testerSt) && s != testerSt)
        {
            return true;
        }
    }
    return false;
}

bool FCLecturer::precarious(void)
{
    for (int i = 0; i < projects.size(); i++)
    {
        if (projects.at(i)->precarious()) {
            return true;
        }
    }
    return false;
}

FCStudent* FCLecturer::metaPrefers(FCStudent* s, FCProject* p)
{
    QList<FCStudent*> assignedStudents = QList<FCStudent*>();

    if (p != nullptr) {
        for (int i = 0; i < p->students.size(); i++) {
            assignedStudents.push_back(p->students[i]);
        }
    }
    else
    {
        for (int i = 0; i < projects.size(); i++)
        {
            for (int j = 0; j < projects.at(i)->students.size(); j++)
            {
                assignedStudents.push_back(projects.at(i)->students.at(j));
            }
        }
    }

    int rankOfWorst = -1;
    QList<FCStudent*> worstStudents = QList<FCStudent*>();
    for (int i = 0; i < assignedStudents.size(); i++) {
        FCStudent* tempS = assignedStudents.at(i);
        int rank = getRank(tempS);
        if (rank > rankOfWorst) {
            rankOfWorst = rank;
            worstStudents = QList<FCStudent*>();
            worstStudents.push_back(tempS);
        }
        else if (rank == rankOfWorst) {
            worstStudents.push_back(tempS);
        }
    }

    int rankOfS = getRank(s);
    if (rankOfS > rankOfWorst) {
        return nullptr;
    }
    else if (rankOfS < rankOfWorst) {
        return worstStudents.at(0);
    }
    else {
        if (worstStudents.contains(s)) {
            return s;
        }

        if (s->phase == 2)
        {
            bool worstInPhase1 = false;
            foreach(FCStudent* st, worstStudents)
            {
                if (st->phase != 2) {
                    return st;
                }
            }
        }
    }
    return nullptr;
}

int FCLecturer::getRank(FCStudent* s) {
    int sInd = s->sInd;
    int rank = -1;
    for (int i = 0; i < prefList.length(); i++)
    {
        if (prefList[i]->sInd == sInd) {
            rank = ranks[i];
        }
    }
    return rank;
}

FCStudent* FCLecturer::getPrecarious(void) {
    for(int i = 0; i < projects.size(); i++)
    {
        if (projects.at(i)->precarious()) {
            FCStudent* precSt = projects.at(i)->getPrecarious();
            if (precSt != nullptr) {
                return precSt;
            }
        }
    }
    return nullptr;
}

// Project.java

FCProject::FCProject(int i, int upperQ)
{
    int numAssigned = 0;
    this->upperQ = upperQ;
    pInd = i;
    pNum = pInd + 1;
    students = QList<FCStudent*>();
}

bool FCProject::precarious(void)
{
    for (int i = 0; i < students.size(); i++) {
        if (students.at(i)->isPrecarious()) {
            return true;
        }
    }
    return false;
}

FCStudent* FCProject::getPrecarious(void)
{
    for (int j = 0; j < students.size(); j++) {
        if (students.at(j)->isPrecarious()) {
            return students.at(j);
        }
    }

    return nullptr;
}

void FCProject::addStudent(FCStudent* s)
{
    int pnum = pInd + 1;
    int snum = s->sInd + 1;

    students.push_back(s);
    s->assigned(this);
    lec->numAssigned++;
    numAssigned++;
}

void FCProject::removeStudent(FCStudent* s)
{
    int pnum = pInd + 1;
    int snum = s->sInd + 1;

    students.removeOne(s);
    s->unassigned();
    lec->numAssigned--;
    numAssigned--;
}

void FCProject::attachLec(FCLecturer* l)
{
    lec = l;
}

bool FCProject::isFullyAvailable(void)
{
    if (numAssigned < upperQ && lec->numAssigned < lec->upperQ) {
        return true;
    }
    else {
        return false;
    }
}

// Model.java

FCModel::FCModel(QList<QList<int>> studentPrefArray, QList<QList<int>> studentPrefRankArray, QList<QList<int>> lecturerPrefArray,
                 QList<QList<int>> lecturerPrefRankArray, QList<int> projectLowerQuotas, QList<int> projUpperQuotas,
                 QList<int> projLecturers, QList<int> lecturerLowerQuotas, QList<int> lecturerUpperQuotas, QList<int> lecturerTargets)
{
    feasible = true;

    this->studentPrefArray = studentPrefArray;
    this->studentPrefRankArray = studentPrefRankArray;
    this->lecturerPrefArray = lecturerPrefArray;
    this->lecturerPrefRankArray = lecturerPrefRankArray;

    this->projectLowerQuotas = projectLowerQuotas;
    this->projUpperQuotas = projUpperQuotas;
    this->projLecturers = projLecturers;

    this->lecturerLowerQuotas = lecturerLowerQuotas;
    this->lecturerUpperQuotas = lecturerUpperQuotas;
    this->lecturerTargets = lecturerTargets;

    numStudents = studentPrefArray.length();
    numProjects = projectLowerQuotas.length();
    numLecturers = lecturerLowerQuotas.length();

    lecturersProjs = QList<QList<int>>();
    for (int z = 0; z < numLecturers; z++) {
        QList<int> lecturerProj = QList<int>();
        for (int plIndex = 0; plIndex < projLecturers.length(); plIndex++) {
            if (projLecturers[plIndex] == z) {
                lecturerProj.push_back(plIndex);
            }
        }
        lecturersProjs.push_back(lecturerProj);
    }

    studentAssignments = QList<int>(numStudents);
    for (int i = 0; i < numStudents; i++) {
        studentAssignments[i] = -1;
    }

    maxPrefLength = 0;
    for (int i = 0; i < studentPrefArray.length(); i++) {
        if (studentPrefArray[i].length() > maxPrefLength) {
            maxPrefLength = studentPrefArray[i].length();
        }
    }
    infoString = "";
    timeAndDate = "";
}

QString FCModel::getNoMatchingResult(void)
{
    QString returnString = "";

    returnString += timeAndDate;

    returnString += "No matching found.";

    return returnString;
}

QString FCModel::getRawResults(void)
{
    QString returnString = "";

    for (int i = 0; i < numStudents; i++) {
        int project = studentAssignments[i] + 1;
        returnString += QString::number(project) + " ";
    }
    return returnString;
}

QString FCModel::getStringAssignments(void)
{
    QString s = "";
    for (int i = 0; i < studentAssignments.length(); i++)
    {
        s += QString::number(studentAssignments[i]) + " ";
    }
    return s;
}

void FCModel::setAssignmentInfo(void)
{
    projectAssignments = QList<QList<int>>();
    for(int i = 0; i < numProjects; i++)
    {
        projectAssignments.push_back(QList<int>());
    }

    numProjectAssignments = QList<int>(numProjects, 0);
    numLecturerAssignments = QList<int>(numLecturers, 0);

    for (int i = 0; i < numStudents; i++)
    {
        int studentNum = i+1;
        int project = studentAssignments[i];
        if (project != -1) {
            projectAssignments[project].push_back(i);
        }
    }

    for (int i = 0; i < numLecturers; i++) {
        int numLecAssignments = 0;
        foreach (int proj, lecturersProjs.at(i))
        {
            numLecAssignments += projectAssignments.at(proj).size();
        }
        numLecturerAssignments[i] = numLecAssignments;
    }
}

QString FCModel::getAllResults(void)
{
    setAssignmentInfo();

    QString assignmentsString = "";

    assignmentsString += "# details of which project each student is assigned \n";
    assignmentsString += "Student_assignments:\n";
    for (int i = 0; i < numStudents; i++)
    {
        int studentNum = i+1;
        assignmentsString += "s_" + QString::number(studentNum) + ":";
        int project = studentAssignments[i];
        if (project == -1) {
            assignmentsString += " no assignment\n";
        }
        else {
            int pl = projLecturers[project];
            assignmentsString += " p_" + QString::number(project) + " (l_" + QString::number(pl) + ")" + "\n";
        }
    }
    assignmentsString += "\n";

    assignmentsString += "# details of which students each project is assigned, and the number of students\n" ;
    assignmentsString += "# assigned compared to the projects maximum capacity \n";
    assignmentsString += "Project_assignments:\n";
    for (int i = 0; i < numProjects; i++) {
        int pl = projLecturers[i];
        int projectNum = i+1;
        assignmentsString += "p_" + QString::number(projectNum) + " (l_" + QString::number(pl) + "): ";

        if (numProjectAssignments[i] == 0) {
            assignmentsString += "no assignment";
        }

        foreach(int j, projectAssignments.at(i)) {
            assignmentsString += "s_" + QString::number(j) + " ";
        }
        assignmentsString += "   " + QString::number(numProjectAssignments[i]) + "/" + QString::number(projUpperQuotas[i]) + "\n";
    }
    assignmentsString += "\n";

    assignmentsString += "# details of which students each lecturer is assigned, and the number of students\n";
    assignmentsString += "# assigned compared to the lecturers maximum capacity (target in brackets) \n";
    assignmentsString += "Lecturer_assignments:\n";
    for (int i = 0; i < numLecturers; i++)
    {
        int lecturerNum = i + 1;
        assignmentsString += "l_" + QString::number(lecturerNum) + ": ";

        foreach (int proj, lecturersProjs.at(i)) {

            foreach (int j, projectAssignments.at(proj)) {
                int projNum = proj + 1;
                assignmentsString += "s_" + QString::number(j) + " (p_" + QString::number(projNum) + ") ";
            }
        }
        if (numLecturerAssignments[i] == 0) {
            assignmentsString += "no assignment";
        }
        assignmentsString += "   " + QString::number(numLecturerAssignments[i]) + "/" + QString::number(lecturerUpperQuotas[i]) + "   (" + QString::number(lecturerTargets[i]) + ")\n";
    }

    QString returnString = "";

    returnString += timeAndDate;
    returnString += infoString + "\n";

    int size = getMatchingSize();
    returnString += "# the number of students matched compared with the total number of students\n";
    returnString += "Matching_size: " + QString::number(size) + "/" + QString::number(numStudents) + "\n\n";

    int matchedCost = calcMatchedCost();
    returnString += "# the sum of ranks of student assignments in the matching\n";
    returnString += "Matching_cost: " + QString::number(matchedCost) + "\n\n";

    int maxDiff = calcMaxAbsDiffLec();
    returnString += "# the maximum absolute difference between the number of students assigned to a lecturer and their target\n";
    returnString += "Matching_max_lecturer_abs_diff: " + QString::number(maxDiff) + "\n\n";

    int lecDiffs = sumAbsLecDiffs();
    returnString += "# the sum the difference between the number of students assigned to a lecturer and their target\n";
    returnString += "Sum_lecturer_abs_diff: " + QString::number(lecDiffs) + "\n\n";

    int lecSqDiffs = sumSqAbsLecDiffs();
    returnString += "# the sum of squares of the difference between the number of students assigned to a lecturer and their target\n";
    returnString += "Sum_sq_lecturer_abs_diff: " + QString::number(lecSqDiffs) + "\n\n";

    int sumSqRanks = calcSumSqRanks();
    returnString += "# the sum of squares of the ranks\n";
    returnString += "Sum_sq_ranks: " + QString::number(sumSqRanks) + "\n\n";

    int sumSqDiffsAndRanks = calcSumSqDiffsAndRanks();
    returnString += "# the sum of squares of the lecturer diffs and ranks\n";
    returnString += "Sum_sq_diffs_and_ranks: " + QString::number(sumSqDiffsAndRanks) + "\n\n";

    QString profile = getMatchingProfile();
    returnString += "# the number of students gaining their 1st, 2nd, 3rd choice project etc\n";
    returnString += "Matching_profile: " + profile + "\n\n";

    int matchedDegree = calcMatchedDegree();
    returnString += "# the highest rank of project assigned to a student in the matching\n";
    returnString += "Matching_degree: " + QString::number(matchedDegree) + "\n\n";

    // add the assignments string to the return string and return
    returnString += assignmentsString;
    return returnString;
}

QString FCModel::getBriefStatsFeasible(void)
{
    QString returnString = "";

    // add information relating to which optimisations have taken place
    returnString += "# " + timeAndDate;
    returnString += infoString + "\n";

    // add further matching statistics
    returnString += "Instance_feasible \n";

    int size = getMatchingSize();
    returnString += "Matching_size: " + QString::number(size) + "/" + QString::number(numStudents) + "\n";

    int matchedCost = calcMatchedCost();
    returnString += "Matching_cost: " + QString::number(matchedCost) + "\n";

    int maxDiff = calcMaxAbsDiffLec();
    returnString += "Matching_max_lecturer_abs_diff: " + QString::number(maxDiff) + "\n";

    int lecDiffs = sumAbsLecDiffs();
    returnString += "Sum_lecturer_abs_diff: " + QString::number(lecDiffs) + "\n";

    int lecSqDiffs = sumSqAbsLecDiffs();
    returnString += "Sum_sq_lecturer_abs_diff: " + QString::number(lecSqDiffs) + "\n";

    int sumSqRanks = calcSumSqRanks();
    returnString += "Sum_sq_ranks: " + QString::number(sumSqRanks) + "\n";

    int sumSqDiffsAndRanks = calcSumSqDiffsAndRanks();
    returnString += "Sum_sq_diffs_and_ranks: " + QString::number(sumSqDiffsAndRanks) + "\n";

    QString profile = getMatchingProfile();
    returnString += "Matching_profile: " + profile + "\n";

    int matchedDegree = calcMatchedDegree();
    returnString += "Matching_degree: " + QString::number(matchedDegree);

    return returnString;
}

QString FCModel::getBriefStatsInfeasible(void)
{
    QString returnString = "";
    returnString += "# " + timeAndDate;

    returnString += infoString + "\n";

    returnString += "Instance_infeasible \n";
    return returnString;
}

int FCModel::sumSqAbsLecDiffs(void)
{
    QList<int> diffs = getAbsDiffs();

    int sumSq = 0;

    for (int z = 0; z < diffs.length(); z++) {
        sumSq += diffs[z] * diffs[z];
    }

    return sumSq;
}

int FCModel::sumAbsLecDiffs(void)
{
    QList<int> diffs = getAbsDiffs();

    int sum = 0;

    for (int z = 0; z < diffs.length(); z++)
    {
        sum += diffs[z];
    }

    return sum;
}

QList<int> FCModel::getAbsDiffs(void)
{
    QList<int> diffs = QList<int>(numLecturers, 0);

    for (int z = 0; z < numLecturers; z++) {
        int targetMinusNum = lecturerTargets[z] - numLecturerAssignments[z];
        int numMinusTarget = numLecturerAssignments[z] - lecturerTargets[z];

        int absDiff = 0;
        if (targetMinusNum < numMinusTarget) {
            absDiff = numMinusTarget;
        }
        else
        {
            absDiff = targetMinusNum;
        }

        diffs[z] = absDiff;
    }
    return diffs;
}

int FCModel::calcMatchedCost(void)
{
    int cost = 0;
    for (int i = 0; i < numStudents; i++) {
        if (studentAssignments[i] != -1) {
            cost += getRank(i, studentAssignments[i]);
        }
    }
    return cost;
}

int FCModel::calcSumSqRanks(void)
{
    int sumSqRanks = 0;
    for (int i = 0; i < numStudents; i++) {
        if (studentAssignments[i] != -1) {
            sumSqRanks += getRank(i, studentAssignments[i]) * getRank(i, studentAssignments[i]);
        }
    }
    return sumSqRanks;
}

int FCModel::calcSumSqDiffsAndRanks(void) {
    int sumSqDiffs = sumSqAbsLecDiffs();
    int sumSqRanks = calcSumSqRanks();
    return sumSqDiffs + sumSqRanks;
}

int FCModel::calcMaxAbsDiffLec(void)
{
    int maxDiff = -1;
    for (int k = 0; k < numLecturers; k++) {
        int targetMinusAssigned = lecturerTargets[k] - numLecturerAssignments[k];
        int assignedMinusTarget = numLecturerAssignments[k] - lecturerTargets[k];
        if (maxDiff < targetMinusAssigned) {
            maxDiff = targetMinusAssigned;
        }
        if (maxDiff < assignedMinusTarget) {
            maxDiff = assignedMinusTarget;
        }
    }
    return maxDiff;
}

QList<int> FCModel::calcProfile(void)
{
    QList<int> profile = QList<int>(maxPrefLength, 0);
    for (int i = 0; i < numStudents; i++) {
        if (studentAssignments[i] != -1) {
            int rank = getRank(i, studentAssignments[i]);
            profile[rank - 1]++;
        }
    }
    return profile;
}

int FCModel::getRank(int studentInd, int projectInd) {
    QList<int> studentPref = studentPrefArray[studentInd];
    for(int i = 0; i < studentPref.length(); i++)
    {
        if (studentPref[i] == projectInd)
        {
            return studentPrefRankArray[studentInd][i];
        }
    }
    return -1;
}

int FCModel::calcMatchedDegree(void)
{
    int matchedDegree = -1;
    QList<int> profile = calcProfile();

    for (int i = 0; i < profile.length(); i++)
    {
        if (profile[i] != 0) {
            matchedDegree = i + 1;
        }
    }
    return matchedDegree;
}

QString FCModel::getMatchingProfile(void)
{
    QList<int> profile = calcProfile();
    QString stringProfile = "< ";
    for (int i = 0; i < profile.length(); i++)
    {
        stringProfile += QString::number(profile[i]) + " ";
    }
    stringProfile += ">";
    return stringProfile;
}

int FCModel::getMatchingSize(void)
{
    int numMatchedStudents = 0;
    for (int i = 0; i < studentAssignments.length(); i++)
    {
        if (studentAssignments[i] != -1) {
            numMatchedStudents++;
        }
    }

    return numMatchedStudents;
}

void FCModel::setInfoString(QString s)
{
    infoString = s;
}

// Util_FileIO.java

FCModel* FCUtil_FileIO::readFile(QFile* file)
{
    bool incLecLQ = true;
    bool incLecTargets = true;
    bool incLecUQ = true;
    bool incProjLQ = true;
    bool incProjUQ = true;

    int numStudents = 0;
    int numProjects = 0;
    int numLecturers = 0;

    QList<QList<int>> studentPrefs(1, QList<int>(1));
    QList<QList<int>> studentPrefRanks(1, QList<int>(1));
    QList<QList<int>> lecturerPrefs(1, QList<int>(1));
    QList<QList<int>> lecturerPrefRanks(1, QList<int>(1));

    QList<int> projectLowerQuotas(1);
    QList<int> projectCapacities(1);
    QList<int> projectLecturers(1);
    QList<int> lecturerUpperQuotas(1);
    QList<int> lecturerLowerQuotas(1);
    QList<int> lecturerTargets(1);
    // maxPrefLength = -1;


    QStringList list = QString(file->readLine()).split(u' ', Qt::SkipEmptyParts);

    numStudents = list[0].toInt();
    numLecturers = list[1].toInt();
    numProjects = list[2].toInt();

    studentPrefs = QList<QList<int>>(numStudents, QList<int>());
    studentPrefRanks = QList<QList<int>>(numStudents, QList<int>());
    lecturerPrefs = QList<QList<int>>(numLecturers, QList<int>());
    lecturerPrefRanks = QList<QList<int>>(numLecturers, QList<int>());

    projectLowerQuotas = QList<int>(numProjects);
    projectCapacities = QList<int>(numProjects);
    projectLecturers = QList<int>(numProjects);
    lecturerUpperQuotas = QList<int>(numLecturers);
    lecturerLowerQuotas = QList<int>(numLecturers);
    lecturerTargets = QList<int>(numLecturers);

    // students
    for (int i = 0; i < numStudents; i++) {
        QString prefList = file->readLine();
        QList<QList<int>> studentPrefsAndRanks = FCUtil_FileIO::getPrefsAndRanks(prefList, 1);
        studentPrefs[i] = studentPrefsAndRanks[0];
        studentPrefRanks[i] = studentPrefsAndRanks[1];
    }

    // lecturers
    for (int i = 0; i < numLecturers; i++) {
        QString s = file->readLine();
        QList<QString> ssplit = s.split(u' ', Qt::SkipEmptyParts);

        QList<QList<int>> lecturerPrefsAndRanks;

        lecturerLowerQuotas[i] = 0;
        lecturerUpperQuotas[i] = ssplit[0].split(u':', Qt::SkipEmptyParts)[1].toInt();
        lecturerTargets[i] = lecturerUpperQuotas[i];

        lecturerPrefsAndRanks = FCUtil_FileIO::getPrefsAndRanks(s, 1);
        lecturerPrefs[i] = lecturerPrefsAndRanks[0];
        lecturerPrefRanks[i] = lecturerPrefsAndRanks[1];
    }

    // projects
    for (int i = 0; i < numProjects; i++) {
        QString s = file->readLine();
        QList<QString> ssplit = s.split(u' ', Qt::SkipEmptyParts);

        projectLowerQuotas[i] = 0;
        projectCapacities[i] = ssplit[0].split(u':', Qt::SkipEmptyParts)[1].toInt();
        projectLecturers[i] = ssplit[1].toInt() - 1;
    }


    FCModel* model = new FCModel(studentPrefs, studentPrefRanks, lecturerPrefs, lecturerPrefRanks,
                projectLowerQuotas, projectCapacities, projectLecturers, lecturerLowerQuotas, lecturerUpperQuotas, lecturerTargets);

    return model;
}

QList<QList<int>> FCUtil_FileIO::getPrefsAndRanks(QString stringPrefs, int startingPosition) {
    // Rewritten from the original

    QList<QString> studentSplit = stringPrefs.split(u' ', Qt::SkipEmptyParts);

    QList<QList<int>> prefAndRankList(2, QList<int>(studentSplit.length() - startingPosition));
    studentSplit.pop_front();

    int index = 0;
    foreach(QString x, studentSplit)
    {
        QStringList pref = x.split(u':', Qt::SkipEmptyParts);
        int prefProj = pref[0].toInt() - 1;
        int prefRank = pref[1].toInt();
        prefAndRankList[0][index] = prefProj;
        prefAndRankList[1][index] = prefRank;
        index++;
    }

    return prefAndRankList;

}

// Approx.java

FCApprox::FCApprox(FCModel* model)
{
    trace = false;
    this->model = model;
    run();
}

void FCApprox::run(void)
{
    QString traceString = "";

    projects = QList<FCProject*>(model->numProjects);
    for (int i = 0; i < projects.length(); i++)
    {
        projects[i] = new FCProject(i, model->projUpperQuotas[i]);
    }

    students = QList<FCStudent*>(model->numStudents);
    for (int i = 0; i < students.length(); i++)
    {

        QList<FCProject*> prefListP(model->studentPrefArray[i].length());
        for (int j = 0; j < prefListP.length(); j++) {
            prefListP[j] = projects[model->studentPrefArray[i][j]];
        }

        students[i] = new FCStudent(i, prefListP, model->studentPrefRankArray[i]);
    }

    lecturers = QList<FCLecturer*>(model->numLecturers);
    for (int i = 0; i < lecturers.length(); i++)
    {
        QList<FCStudent*> prefListL(model->lecturerPrefArray[i].length());
        for (int j = 0; j < prefListL.length(); j++) {
            prefListL[j] = students[model->lecturerPrefArray[i][j]];
        }
        lecturers[i] = new FCLecturer(i, prefListL, model->lecturerPrefRankArray[i], model->lecturerUpperQuotas[i]);
    }

    for (int i = 0; i < model->projLecturers.length(); i++)
    {
        FCProject* proj = projects[i];
        FCLecturer* lec = lecturers[model->projLecturers[i]];
        proj->attachLec(lec);
        lec->attachProj(proj);
    }

    QList<FCStudent*> phase1or2Avaliable = QList<FCStudent*>();
    for (int i = 0; i < students.length(); i++)
    {
        phase1or2Avaliable.push_back(students[i]);
    }
    int numAvaliable = model->numStudents;

    // Algorithm proper

    while (phase1or2Avaliable.size() != 0) {

        FCStudent* s = phase1or2Avaliable.at(0);
        int pInd = s->getFavourite(projects);
        FCProject* p = projects[pInd];
        int lInd = model->projLecturers[pInd];
        FCLecturer* l = lecturers[lInd];

        traceString += "student " + QString::number(s->sNum) + " applies to project " + QString::number(p->pNum) + ":\n";

        if (p->isFullyAvailable()) {
            traceString += "   - algorithm placement information: project " + QString::number(p->pNum) + " is fully available\n";
            traceString += "   - pair (" + QString::number(s->sNum) + ", " + QString::number(p->pNum) + ") added\n";
            p->addStudent(s);
            phase1or2Avaliable.remove(phase1or2Avaliable.indexOf(s));
        }

        else if ((p->numAssigned < p->upperQ) && (l->numAssigned == l->upperQ) && (l->precarious() || (l->metaPrefers(s) != nullptr))) {
            traceString += "   - algorithm placement information: project " + QString::number(p->pNum) + " is undersubscribed, lecturer " + QString::number(l->lNum) + " is full\n";
            FCStudent* stReplaceMe = nullptr;

            if (l->precarious()) {
                stReplaceMe = l->getPrecarious();
                traceString += "   - student " + QString::number(stReplaceMe->sNum) + " is found as precarious\n";
            }
            else {
                stReplaceMe = l->metaPrefers(s);
                traceString += "   - student " + QString::number(stReplaceMe->sNum) + " is a worst student for lecturer " + QString::number(l->lNum) + "\n";
                traceString += "   - student " + QString::number(stReplaceMe->sNum) + " removes project " + QString::number(stReplaceMe->proj->pNum) + " from pref list - student " + QString::number(stReplaceMe->sNum) + " is in phase: " + QString::number(stReplaceMe->phase) + "\n";
                stReplaceMe->removePref(stReplaceMe->proj);
            }

            FCProject* replaceP = stReplaceMe->proj;
            replaceP->removeStudent(stReplaceMe);
            traceString += "   - pair (" + QString::number(stReplaceMe->sNum) + ", " + QString::number(replaceP->pNum) + ") removed\n";

            p->addStudent(s);
            traceString += "   - pair (" + QString::number(s->sNum) + ", " + QString::number(p->pNum) + ") added\n";

            phase1or2Avaliable.remove(phase1or2Avaliable.indexOf(s));
            if (stReplaceMe->phase != 3) {
                phase1or2Avaliable.push_back(stReplaceMe);
            }
        }

        else if ((p->numAssigned == p->upperQ) && (p->precarious() || l->metaPrefers(s, p) != nullptr)) {
            traceString += "   - algorithm placement information: project " + QString::number(p->pNum) + " is full\n";
            FCStudent* stReplaceMe = nullptr;

            if (p->precarious()) {
                stReplaceMe = p->getPrecarious();
                traceString += "   - student " + QString::number(stReplaceMe->sNum) + " is found as precarious\n";
            }
            else {
                stReplaceMe = l->metaPrefers(s, p);
                stReplaceMe->removePref(stReplaceMe->proj);
                traceString += "   - student " + QString::number(stReplaceMe->sNum) + " is a worst student for project " + QString::number(p->pNum) + "\n";
                traceString += "   - student " + QString::number(stReplaceMe->sNum) + " removes project " + QString::number(p->pNum) + " from pref list - student " + QString::number(stReplaceMe->sNum) + " is in phase: " + QString::number(stReplaceMe->phase) + "\n";
            }

            FCProject* replaceP = stReplaceMe->proj;
            replaceP->removeStudent(stReplaceMe);
            traceString += "   - pair (" + QString::number(stReplaceMe->sNum) + ", " + QString::number(replaceP->pNum) + ") removed\n";

            p->addStudent(s);
            traceString += "   - pair (" + QString::number(s->sNum) + ", " + QString::number(p->pNum) + ") added\n";

            phase1or2Avaliable.remove(phase1or2Avaliable.indexOf(s));
            if (stReplaceMe->phase != 3) {
                phase1or2Avaliable.push_back(stReplaceMe);
            }
        }

        else {
            s->removePref(p);
            traceString += "   - algorithm placement information: student " + QString::number(s->sNum) + " rejected\n";
            traceString += "   - student " + QString::number(s->sNum) + " removes project " + QString::number(p->pNum) + " from pref list - student " + QString::number(s->sNum) + " is in phase: " + QString::number(s->phase) + "\n";
            if (s->phase == 3) {
                phase1or2Avaliable.remove(phase1or2Avaliable.indexOf(s));
            }
        }
    }

    bool finishedPromoting = false;
    while (!finishedPromoting) {

        bool promoteInIteration = false;
        foreach (FCStudent* s, students) {
            if (s->proj != nullptr) {
                FCProject* currentProj = s->proj;
                QList<FCProject*> ps = s->getPreferredProjs();
                int index = 0;
                bool found = false;
                while (!found && index < ps.size()) {
                    FCProject* testerProj = ps.at(index);
                    if (testerProj->numAssigned < testerProj->upperQ && testerProj->lec == currentProj->lec) {
                        traceString += "Promoting student " + QString::number(s->sNum) + "\n";
                        traceString += "   - algorithm placement information: promoting students\n";
                        traceString += "   - pair (" + QString::number(s->sNum) + ", " + QString::number(s->proj->pNum) + ") removed\n";
                        s->proj->removeStudent(s);
                        testerProj->addStudent(s);
                        found = true;
                        promoteInIteration = true;
                        traceString += "   - pair (" + QString::number(s->sNum) + ", " + QString::number(s->proj->pNum) + ") added\n";
                    }
                    index++;
                }
            }
        }
        if (!promoteInIteration) {
            finishedPromoting = true;
        }
    }

    for (int i = 0; i < students.length(); i++) {
        int finalProj = -1;
        if (students[i]->proj != nullptr) {
            finalProj = students[i]->proj->pInd;
        }
        model->studentAssignments[i] = finalProj;
    }

    if (trace) {
        // add output print functionality
    }

}
