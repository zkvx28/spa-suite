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

QList<Supervisor*> supervisors;
QMap<int, Supervisor*> projects; // Map project ID to supervisor
QList<Student*> students;
QHash<Student*, int> spa; // Matching of student (key) to project (value)

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
                    QList<int> prefs;
                    foreach(QString n, list)
                    {
                        if (prefs.contains(n.toInt()))
                        {
                            QMessageBox error;
                            error.setText("Error: duplicated prefs on line "+QString::number(i));
                            error.exec();
                            MainWindow::freeAndClearSupervisors();
                            return;
                        }
                        prefs.append(n.toInt());
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

            if (!projects.isEmpty() && students.count() > 0 && supervisors.count() > 0)
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
                    QList<int> prefs;
                    foreach(QString n, list)
                    {
                        if (prefs.contains(n.toInt()))
                        {
                            QMessageBox error;
                            error.setText("Error: duplicated prefs on line "+QString::number(i));
                            error.exec();
                            MainWindow::freeAndClearStudents();
                            return;
                        }
                        prefs.append(n.toInt());
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

            if (!projects.isEmpty() && students.count() > 0 && supervisors.count() > 0)
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

                    projects[i] = supervisors[supervisor];
                }
                i++;
            }

            // Load file name into label
            QString fnOut = fileNames[0];
            fnOut.truncate(40);
            fnOut.append("...");
            ui->fnProjectsLabel->setText(fnOut);

            if (!projects.isEmpty() && students.count() > 0 && supervisors.count() > 0)
            {
                ui->startButton->setEnabled(true);
            }
        }
    }
}
