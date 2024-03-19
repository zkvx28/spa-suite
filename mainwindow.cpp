#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QByteArray>
#include <QErrorMessage>
#include <QtLogging>
#include "spa.h"
#include "datagenwindow.h"
#include "./ui_datagenwindow.h"
#include "cooper.h"

QString datasetPath;

QList<Supervisor*> supervisors;
QList<Project*> projects; // Map project ID to supervisor
QList<Student*> students;
int noProjects;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*void MainWindow::freeAndClearSupervisors()
{
    ui->fnSupervisorsLabel->setText("No file loaded");
    for(int j = 0; j < supervisors.count(); j++)
    {
        free(supervisors[j]);
    }
    supervisors.clear();
}

void MainWindow::freeAndClearStudents()
{
    ui->fnStudentsLabel->setText("No file loaded");
    for(int j = 0; j < students.count(); j++)
    {
        free(students[j]);
    }
    students.clear();
}

void MainWindow::on_loadSupervisorsButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Text files (*.txt)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        freeAndClearSupervisors();

        fileNames = dialog.selectedFiles();
        if (!(fileNames.empty()))
        {
            // Load supervisor data
            QFile file(fileNames[0]);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return;

            int i = 0;
            while (!file.atEnd())
            {
                QString line = QString(file.readLine());
                QStringList list = line.split(u' ', Qt::SkipEmptyParts);
                if (!list.empty())
                {
                    QList<QString> base = list[0].split(u':', Qt::SkipEmptyParts);
                    if (base.count() != 2)
                    {
                        QMessageBox error;
                        error.setText("Error: malformed ID on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        freeAndClearSupervisors();
                        return;
                    }
                    int id = base[0].toInt();
                    if (id != i)
                    {
                        QMessageBox error;
                        error.setText("Error: incorrect ID on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        freeAndClearSupervisors();
                        return;
                    }
                    int capacity = base[1].toInt();
                    if (capacity < 1)
                    {
                        QMessageBox error;
                        error.setText("Error: incorrect capacity on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        freeAndClearSupervisors();
                        return;
                    }
                    list.removeFirst();
                    PrefsMap prefs;
                    foreach(QString n, list)
                    {
                        QList<QString> cur = n.split(u':', Qt::SkipEmptyParts);
                        if (prefs.contains(cur[0].toInt()))
                        {
                            QMessageBox error;
                            error.setText("Error: duplicated prefs on line "+QString::number(i));
                            error.exec();
                            MainWindow::freeAndClearSupervisors();
                            return;
                        }
                        if (cur[1].toInt() <= 0)
                            prefs[cur[0].toInt()] = -1;
                        else
                            prefs[cur[0].toInt()] = cur[1].toInt();
                    }
                    supervisors.append(new Supervisor(id, prefs, capacity));
                }
                i++;
            }

            // Load file name into label
            QString fnOut = fileNames[0];
            fnOut.truncate(40);
            fnOut.append("...");
            ui->fnSupervisorsLabel->setText(fnOut);

            // Enable projects button now supervisors loaded
            ui->loadProjectsButton->setEnabled(true);

            if (projects.count() > 0 && students.count() > 0 && supervisors.count() > 0)
            {
                ui->startButton->setEnabled(true);
                ui->cooperStartButton->setEnabled(true);
            }

            //ui->fnSupervisorsLabel->setText("Supervisors count: "+QString::number(supervisors.count()));
        }
    }
}

void MainWindow::on_loadStudentsButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Text files (*.txt)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        MainWindow::freeAndClearStudents();

        fileNames = dialog.selectedFiles();
        if (!(fileNames.empty()))
        {
            // Load student data
            QFile file(fileNames[0]);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return;

            int i = 0;
            while (!file.atEnd())
            {
                QString line = QString(file.readLine());
                QStringList list = line.split(u' ', Qt::SkipEmptyParts);
                if (!list.empty())
                {
                    int id = list[0].toInt();
                    if (id != i)
                    {
                        QMessageBox error;
                        error.setText("Error: incorrect ID on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        MainWindow::freeAndClearStudents();
                        return;
                    }
                    list.removeFirst();
                    PrefsMap prefs;
                    foreach(QString n, list)
                    {
                        QList<QString> cur = n.split(u':', Qt::SkipEmptyParts);
                        if (prefs.contains(cur[0].toInt()))
                        {
                            QMessageBox error;
                            error.setText("Error: duplicated prefs on line "+QString::number(i) + " ("+QString::number(cur[0].toInt())+")");
                            error.exec();
                            MainWindow::freeAndClearStudents();
                            return;
                        }
                        if (cur[1].toInt() <= 0)
                            prefs[cur[0].toInt()] = -1;
                        else
                            prefs[cur[0].toInt()] = cur[1].toInt();
                    }
                    students.append(new Student(id, prefs));
                    prefs.clear();
                }
                i++;
            }

            // Load file name into label
            QString fnOut = fileNames[0];
            fnOut.truncate(40);
            fnOut.append("...");
            ui->fnStudentsLabel->setText(fnOut);

            if (projects.count() > 0 && students.count() > 0 && supervisors.count() > 0)
            {
                ui->startButton->setEnabled(true);
                ui->cooperStartButton->setEnabled(true);
            }
        }
    }
}


void MainWindow::on_loadProjectsButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Text files (*.txt)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        projects.clear();

        fileNames = dialog.selectedFiles();
        if (!(fileNames.empty()))
        {
            // Load project data
            QFile file(fileNames[0]);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return;

            int i = 0;
            while (!file.atEnd())
            {
                QString line = QString(file.readLine());
                QStringList list = line.split(u' ', Qt::SkipEmptyParts);
                if (!list.empty())
                {
                    if (list.count() != 2)
                    {
                        QMessageBox error;
                        error.setText("Error: invalid format on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        projects.clear();
                        return;
                    }
                    QList<QString> base = list[0].split(u':', Qt::SkipEmptyParts);
                    if (base.count() != 2)
                    {
                        QMessageBox error;
                        error.setText("Error: malformed ID on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        freeAndClearSupervisors();
                        return;
                    }
                    int id = base[0].toInt();
                    if (id != i)
                    {
                        QMessageBox error;
                        error.setText("Error: incorrect ID on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        freeAndClearSupervisors();
                        return;
                    }
                    int capacity = base[1].toInt();
                    if (capacity < 1)
                    {
                        QMessageBox error;
                        error.setText("Error: incorrect capacity on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        freeAndClearSupervisors();
                        return;
                    }
                    int supervisor = list[1].toInt();
                    if ((supervisor < 0) || (supervisor > supervisors.count()))
                    {
                        QMessageBox error;
                        error.setText("Error: invalid supervisor ID on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        projects.clear();
                        return;
                    }
                    projects.append(new Project(id, supervisors[supervisor], capacity));
                }
                i++;
            }

            // Load file name into label
            QString fnOut = fileNames[0];
            fnOut.truncate(40);
            fnOut.append("...");
            ui->fnProjectsLabel->setText(fnOut);

            if (projects.count() > 0 && students.count() > 0 && supervisors.count() > 0)
            {
                ui->startButton->setEnabled(true);
                ui->cooperStartButton->setEnabled(true);
            }
        }
    }
}*/

bool MainWindow::validateDataset(QString datasetPath)
{
    QFile data(datasetPath);
    QMessageBox error;
    error.setIcon(QMessageBox::Critical);

    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        error.setText("Could not open dataset file at given path.");
        error.exec();
        return false;
    }

    // Check counts of student/supervisor/project

    QStringList list = QString(data.readLine()).split(u' ', Qt::SkipEmptyParts);

    if (list.count() != 3)
    {
        error.setText("Student/supervisor/project counts not defined correctly.");
        error.exec();
        data.close();
        return false;
    }

    for (int i = 0; i < 3; i++)
    {
        if (list[i].toInt() <= 0)
        {
            switch(i)
            {
            case 0:
                error.setText("Student count is invalid.");
                break;
            case 1:
                error.setText("Supervisor count is invalid.");
                break;
            case 2:
                error.setText("Project count is invalid.");
                break;
            }

            error.exec();
            data.close();
            return false;
        }
    }

    int studentCount = list[0].toInt();
    int supervisorCount = list[1].toInt();
    int projectCount = list[2].toInt();

    // Now check the students

    for (int index = 1; index <= studentCount; index++)
    {
        list = QString(data.readLine()).split(u' ', Qt::SkipEmptyParts);
        if (list[0].toInt() != index)
        {
            error.setText("Student "+QString::number(list[0].toInt())+" does not have index "+QString::number(index));
            error.exec();
            data.close();
            return false;
        }
        list.pop_front();
        // Now check the preferences
        int lowestRank = 1;
        int temp = 0;
        foreach(QString x, list)
        {
            QStringList pref = x.split(u':', Qt::SkipEmptyParts);
            int prefProj = pref[0].toInt();
            int prefRank = pref[1].toInt();
            if (prefRank == lowestRank)
                temp++;
            if (prefRank == lowestRank + temp)
            {
                lowestRank = lowestRank + temp;
                temp = 0;
            }
            if (prefProj <= 0 || prefProj > studentCount || prefRank <= 0 || prefRank > lowestRank)
            {
                error.setText("Student "+QString::number(index)+" has an invalid preference");
                error.exec();
                data.close();
                return false;
            }
        }
    }

    // Now check the supervisors

    for (int index = 1; index <= supervisorCount; index++)
    {
        list = QString(data.readLine()).split(u' ', Qt::SkipEmptyParts);
        QStringList v = list[0].split(u':', Qt::SkipEmptyParts);
        if (v[0].toInt() != index)
        {
            error.setText("Supervisor "+QString::number(v[0].toInt())+" does not have index "+QString::number(index));
            error.exec();
            data.close();
            return false;
        }
        if (v[1].toInt() <= 0)
        {
            error.setText("Supervisor "+QString::number(index)+" has invalid capacity");
            error.exec();
            data.close();
            return false;
        }
        list.pop_front();
        // Now check the preferences
        int lowestRank = 1;
        int temp = 0;
        foreach(QString x, list)
        {
            QStringList pref = x.split(u':', Qt::SkipEmptyParts);
            int prefProj = pref[0].toInt();
            int prefRank = pref[1].toInt();
            if (prefRank == lowestRank)
                temp++;
            if (prefRank == lowestRank + temp)
            {
                lowestRank = lowestRank + temp;
                temp = 0;
            }
            if (prefProj <= 0 || prefProj > studentCount || prefRank <= 0 || prefRank > lowestRank)
            {
                error.setText("Supervisor "+QString::number(index)+" has an invalid preference");
                error.exec();
                data.close();
                return false;
            }
        }
    }

    // Now check the projects

    for (int index = 1; index <= projectCount; index++)
    {
        list = QString(data.readLine()).split(u' ', Qt::SkipEmptyParts);
        QStringList p = list[0].split(u':', Qt::SkipEmptyParts);
        if (p[0].toInt() != index)
        {
            error.setText("Project "+QString::number(p[0].toInt())+" does not have index "+QString::number(index));
            error.exec();
            data.close();
            return false;
        }
        if (p[1].toInt() <= 0)
        {
            error.setText("Project "+QString::number(index)+" has invalid capacity");
            error.exec();
            data.close();
            return false;
        }
        list.pop_front();
        // Now check the linked supervisors
        foreach (QString x, list)
        {
            if (x.toInt() <= 0 || x.toInt() > supervisorCount)
            {
                error.setText("Project "+QString::number(index)+" has invalid supervisor");
                error.exec();
                data.close();
                return false;
            }
        }
    }

    data.close();

    return true;
}

void MainWindow::on_startButton_clicked()
{
    // Load dataset

    if (!(validateDataset(datasetPath)))
        return;

    QFile* data = new QFile(datasetPath);

    data->open(QIODevice::ReadOnly | QIODevice::Text);

    GAInstance* ga = new GAInstance(data, ui->popSizeInput->value());

    data->close();

/*    ui->startButton->setEnabled(false);

    QFile out("out.txt");
    out.open(QIODevice::WriteOnly);

    out.write("=== EXECUTION BEGIN ===\n");

    out.write("\n== STUDENTS LIST ==\n");
    foreach(Student* s, students)
    {
        out.write("S");
        out.write(QString::number(s->getId()).toUtf8());
        out.write(" PROJECT PREFS: ");
        for (int i = 0; i < projects.count(); i++)
        {
            out.write(QString::number(i).toUtf8());
            out.write(":");
            out.write(QString::number(s->getPref(i)).toUtf8());
            out.write(" ");
        }
        out.write("\n");
    }


    out.write("\n== SUPERVISORS LIST ==\n");
    foreach(Supervisor* v, supervisors)
    {
        out.write("V");
        out.write(QString::number(v->getId()).toUtf8());
        out.write(":");
        out.write(QString::number(v->getCapacity()).toUtf8());
        out.write(" STUDENT PREFS: ");
        for (int i = 0; i < students.count(); i++)
        {
            out.write(QString::number(i).toUtf8());
            out.write(":");
            out.write(QString::number(v->getPref(i)).toUtf8());
            out.write(" ");
        }
        out.write("\n");
    }

    out.write("\n== PROJECTS LIST ==\n");
    int i = 0;
    foreach(Project* p, projects)
    {
        out.write("P");
        out.write(QString::number(i).toUtf8());
        out.write(":");
        out.write(QString::number(p->getCapacity()).toUtf8());
        out.write(" SUPERVISOR: ");
        out.write(QString::number(p->getSupervisor()->getId()).toUtf8());
        out.write("\n");
        i++;
    }

    //out.write("\n== GENERATING INITIAL POPULATION OF ");
    //out.write(QString::number(ui->popSizeInput->value()).toUtf8());
    //out.write(" ==\n");

    GAInstance* spa = new GAInstance(students, projects, supervisors, ui->popSizeInput->value()); // Generate the GAInstance

    //out.write(spa->getState().toLocal8Bit().data());

    int best = 0;
    int worst = INT_MAX;

    for (int i = 1; i <= ui->maxIterationsInput->value(); i++)
    {
        ui->numberIterationsVal->setText(QString::number(i));

        spa->iterateSPA();

        best = std::max(best, spa->bestFitness());
        worst = std::min(worst, spa->worstFitness());

        ui->worstFitnessVal->setText(QString::number(worst));
        ui->bestFitnessVal->setText(QString::number(best));

        out.write("\n== POPULATION AFTER ");
        out.write(QString::number(i).toUtf8());
        out.write(" ITERATIONS ==\n");
        out.write(spa->getState().toLocal8Bit().data());
        out.write("\n");
    }

    out.write("\n=== EXECUTION COMPLETE ===\n");

    out.close();

    ui->startButton->setEnabled(true);*/
}

void MainWindow::on_actionDataset_Generator_triggered()
{
    auto win = new DataGenWindow();
    win->show();
}


int MainWindow::getAssignableStudent(QList<int> matching, QList<int> studentPhases)
{
    for (int i = 0; i < matching.count(); i++)
    {
        if (matching[i] == -1 && (studentPhases[i] == 1 || studentPhases[i] == 2))
            return i;
    }
    return -1;
}

void MainWindow::on_cooperStartButton_clicked()
{
    ui->cooperStartButton->setEnabled(false);

    if (!(validateDataset(datasetPath)))
    {
        ui->cooperStartButton->setEnabled(true);
        return;
    }

    QFile* data = new QFile(datasetPath);

    data->open(QIODevice::ReadOnly | QIODevice::Text);

    FCModel* model = FCUtil_FileIO::readFile(data);

    FCApprox* alg = new FCApprox(model);

    model->setAssignmentInfo();

    QFile out("outCooper.txt");
    out.open(QIODevice::WriteOnly);
    out.write(model->getRawResults().toStdString().c_str());
    out.close();

    data->close();

    ui->cooperStartButton->setEnabled(true);
}


void MainWindow::on_loadDatasetButton_clicked()
{

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Text files (*.txt)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {

        fileNames = dialog.selectedFiles();
        if (!(fileNames.empty()))
        {
            // Load file name into label
            QString fnOut = fileNames[0];
            fnOut = fnOut.last(40);
            fnOut.prepend("...");
            ui->fnDatasetLabel->setText(fnOut);
            // Load file name into datasetPath
            datasetPath = fileNames[0];
        }
    }

    ui->cooperStartButton->setEnabled(true);
    ui->startButton->setEnabled(true);
}

