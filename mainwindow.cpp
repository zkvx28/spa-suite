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

QList<Supervisor*> supervisors;
QList<Supervisor*> projects; // Map project ID to supervisor
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

void MainWindow::freeAndClearSupervisors()
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
                    int id = list[0].toInt();
                    if (id != i)
                    {
                        QMessageBox error;
                        error.setText("Error: incorrect ID on line "+QString::number(i)+" ("+line+")");
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
                    supervisors.append(new Supervisor(id, prefs));
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
                    int id = list[0].toInt();
                    if (id != i)
                    {
                        QMessageBox error;
                        error.setText("Error: incorrect ID on line "+QString::number(i)+" ("+line+")");
                        error.exec();
                        projects.clear();
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
                    projects.append(supervisors[supervisor]);
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
            }
        }
    }
}

void MainWindow::on_startButton_clicked()
{
    ui->startButton->setEnabled(false);

    //QFile out("out.txt");
    //out.open(QIODevice::WriteOnly);

    //out.write("=== EXECUTION BEGIN ===\n");

    //out.write("\n== STUDENTS LIST ==\n");
    foreach(Student* s, students)
    {
        //out.write("S");
        //out.write(QString::number(s->getId()).toUtf8());
        //out.write(" PROJECT PREFS: ");
        for (int i = 0; i < projects.count(); i++)
        {
            //out.write(QString::number(i).toUtf8());
            //out.write(":");
            //out.write(QString::number(s->getPref(i)).toUtf8());
            //out.write(" ");
        }
        //out.write("\n");
    }


    //out.write("\n== SUPERVISORS LIST ==\n");
    foreach(Supervisor* v, supervisors)
    {
        //out.write("V");
        //out.write(QString::number(v->getId()).toUtf8());
        //out.write(" STUDENT PREFS: ");
        for (int i = 0; i < students.count(); i++)
        {
            //out.write(QString::number(i).toUtf8());
            //out.write(":");
            //out.write(QString::number(v->getPref(i)).toUtf8());
            //out.write(" ");
        }
        //out.write("\n");
    }

    //out.write("\n== PROJECTS LIST ==\n");
    int i = 0;
    foreach(Supervisor* v, projects)
    {
        //out.write("P");
        //out.write(QString::number(i).toUtf8());
        //out.write(" SUPERVISOR: ");
        //out.write(QString::number(v->getId()).toUtf8());
        //out.write("\n");
        i++;
    }

    //out.write("\n== GENERATING INITIAL POPULATION OF ");
    //out.write(QString::number(ui->popSizeInput->value()).toUtf8());
    //out.write(" ==\n");

    SPAInstance* spa = new SPAInstance(students, projects, supervisors, ui->popSizeInput->value()); // Generate the SPAInstance

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

        //out.write("\n== POPULATION AFTER ");
        //out.write(QString::number(i).toUtf8());
        //out.write(" ITERATIONS ==\n");
        //out.write(spa->getState().toLocal8Bit().data());
        //out.write("\n");
    }

    //out.write("\n=== EXECUTION COMPLETE ===\n");

    //out.close();

    ui->startButton->setEnabled(true);
}

void MainWindow::on_actionDataset_Generator_triggered()
{
    auto win = new DataGenWindow();
    win->show();
}

