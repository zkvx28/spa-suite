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
        int tieRank = -1;
        int newRank = 1;
        foreach(QString x, list)
        {
            QStringList pref = x.split(u':', Qt::SkipEmptyParts);
            int prefProj = pref[0].toInt();
            int prefRank = pref[1].toInt();
            if (prefProj <= 0 || prefProj > projectCount || prefRank <= 0 || (prefRank != tieRank && prefRank != newRank))
            {
                error.setText("Student "+QString::number(index)+" has an invalid preference");
                error.exec();
                data.close();
                return false;
            }
            if (prefRank != tieRank)
            {
                tieRank = newRank;
            }
            newRank++;
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
        int tieRank = -1;
        int newRank = 1;
        foreach(QString x, list)
        {
            QStringList pref = x.split(u':', Qt::SkipEmptyParts);
            int prefStud = pref[0].toInt();
            int prefRank = pref[1].toInt();
            if (prefStud <= 0 || prefStud > studentCount || prefRank <= 0 || (prefRank != tieRank && prefRank != newRank))
            {
                error.setText("Supervisor "+QString::number(index)+" has an invalid preference");
                error.exec();
                data.close();
                return false;
            }
            if (prefRank != tieRank)
            {
                tieRank = newRank;
            }
            newRank++;
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

    ui->textBrowser->clear();
    ui->bestFitnessVal->setText("-");
    ui->worstFitnessVal->setText("-");
    ui->convergenceVal->setText("-");
    ui->bestMatchingSizeVal->setText("...");

    GAInstance* ga = new GAInstance(data, ui->popSizeInput->value());

    ga->setMutationRate(ui->mutationRateInput->value());

    data->close();

    disableAllControls();

    QFile out("out.txt");

    if (ui->loggingCheckBox->isChecked())
    {
        out.open(QIODevice::WriteOnly);

/*        out.write("=== EXECUTION BEGIN ===\n");

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

        out.write("\n== GENERATING INITIAL POPULATION OF ");
        out.write(QString::number(ui->popSizeInput->value()).toUtf8());
        out.write(" ==\n");

        out.write(ga->getState().toLocal8Bit().data());*/
    }

    int best = 0;
    int worst = INT_MAX;

    for (int i = 1; i <= ui->maxIterationsInput->value(); i++)
    {
        if (!(i % 10))
            ui->numberIterationsVal->setText(QString::number(i));

        ga->iterateSPA();

        if (ga->bestFitness() > best)
        {
            ui->textBrowser->setText(ga->getBestChromosome());
            ui->convergenceVal->setText(QString::number(i));
            ui->bestMatchingSizeVal->setText("Length: "+QString::number(ga->bestSolutionSize())+"/"+QString::number(ga->students.count()));
            if (ui->loggingCheckBox->isChecked())
            {
                out.write(QString::number(i).toUtf8());
                out.write(": ");
                out.write(QString::number(ga->bestFitness()).toUtf8());
                out.write(", size ");
                out.write(QString::number(ga->bestSolutionSize()).toUtf8());
                out.write("\n");
            }
        }
        best = std::max(best, ga->bestFitness());
        worst = std::min(worst, ga->worstFitness());

        ui->worstFitnessVal->setText(QString::number(worst));
        ui->bestFitnessVal->setText(QString::number(best));
        qApp->processEvents();

        /*if (ui->loggingCheckBox->isChecked())
        {
            out.write("\n== POPULATION AFTER ");
            out.write(QString::number(i).toUtf8());
            out.write(" ITERATIONS ==\n");
            out.write(ga->getState().toLocal8Bit().data());
            out.write("\n");
        }*/
    }

    if (ui->loggingCheckBox->isChecked())
    {
        /*out.write(QString::number(ga->fitness(t)).toUtf8());
        out.write("\n=== EXECUTION COMPLETE ===\n");*/

        out.close();
    }

    enableAllControls();
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
    qApp->processEvents();

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
    out.write("\n");
    data->seek(0);
    GAInstance* ga = new GAInstance(data, ui->popSizeInput->value());
    out.write(QString::number(ga->fitness(model->studentAssignments)).toUtf8());
    out.close();
    data->seek(0);

    data->close();

    delete alg;
    delete model;
    delete ga;
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

void MainWindow::disableAllControls(void)
{
    ui->loadDatasetButton->setEnabled(false);
    ui->popSizeInput->setEnabled(false);
    ui->maxIterationsInput->setEnabled(false);
    ui->loggingCheckBox->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->cooperStartButton->setEnabled(false);
    ui->mutationRateInput->setEnabled(false);
}

void MainWindow::enableAllControls(void)
{
    ui->loadDatasetButton->setEnabled(true);
    ui->popSizeInput->setEnabled(true);
    ui->maxIterationsInput->setEnabled(true);
    ui->loggingCheckBox->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->cooperStartButton->setEnabled(true);
    ui->mutationRateInput->setEnabled(true);
}

