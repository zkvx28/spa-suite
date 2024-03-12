#include "datagenwindow.h"
#include "qmessagebox.h"
#include "ui_datagenwindow.h"
#include <QFile>
#include <QFileDialog>
#include <random>

DataGenWindow::DataGenWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DataGenWindow)
{
    ui->setupUi(this);
}

DataGenWindow::~DataGenWindow()
{
    delete ui;
}

void DataGenWindow::on_pushButton_clicked()
{
    int students = ui->numStudentsVal->value();
    int supervisors = ui->numSupervisorsVal->value();
    int projects = ui->numProjectsVal->value();

    if (projects < students || projects < supervisors)
    {
        QMessageBox error;
        error.setText("Error: invalid project count.\n"
                      "Number of projects must not be less than number of supervisors or students.");
        error.setIcon(QMessageBox::Critical);
        error.setWindowTitle("SPASuite Dataset Generator");
        error.setWindowIcon(QIcon("resource/win_icon.png"));
        error.exec();
        return;
    }

    double incomplete = ui->incompletenessVal->value();
    double ties = ui->tiesVal->value();

    std::mt19937 gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> distSupervisors(0, supervisors-1);
    std::uniform_real_distribution<> distReals(0.0, 1.0);

    // Select save location
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Students Dataset"),
                                                    "students.txt",
                                                    tr("Text Files (*.txt)"));

    QFile outS(fileName);
    if (!outS.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    int pref = 1; // Start with first-choice

    // Build a list of projects & students (will shuffle for preferences later)
    QList<int> tempProjects(projects);
    QList<int> tempStudents(students);
    for (int i = 0; i < projects; i++)
    {
        tempProjects[i] = i;
    }
    for (int i = 0; i < students; i++)
    {
        tempStudents[i] = i;
    }

    // Build students
    for (int i = 0; i < students; i++)
    {
        std::random_shuffle(tempProjects.begin(), tempProjects.end());

        outS.write(QString::number(i).toUtf8());
        for (int j = 0; j < projects; j++)
        {
            if (distReals(gen) >= incomplete) // skip assignment if lower than incomplete rate
            {
                outS.write(" ");
                outS.write(QString::number(tempProjects[j]).toUtf8());
                outS.write(":");
                outS.write(QString::number(pref).toUtf8());

                if (distReals(gen) >= ties)
                {
                    pref++;
                }
            }
        }
        outS.write("\n");
        pref = 1;
    }

    outS.close();

    // Select save location
    fileName = QFileDialog::getSaveFileName(this, tr("Save Supervisors Dataset"),
                                                    "supervisors.txt",
                                                    tr("Text Files (*.txt)"));

    QFile outV(fileName);
    if (!outV.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    // Build supervisors
    for (int i = 0; i < supervisors; i++)
    {
        std::random_shuffle(tempStudents.begin(), tempStudents.end());

        outV.write(QString::number(i).toUtf8());
        for (int j = 0; j < students; j++)
        {
            if (distReals(gen) >= incomplete) // skip assignment if lower than incomplete rate
            {
                outV.write(" ");
                outV.write(QString::number(tempStudents[j]).toUtf8());
                outV.write(":");
                outV.write(QString::number(pref).toUtf8());

                if (distReals(gen) >= ties)
                {
                    pref++;
                }
            }
        }
        outV.write("\n");
        pref = 1;
    }

    outV.close();

    // Select save location
    fileName = QFileDialog::getSaveFileName(this, tr("Save Projects Dataset"),
                                            "projects.txt",
                                            tr("Text Files (*.txt)"));

    QFile outP(fileName);
    if (!outP.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    // Build projects
    for (int i = 0; i < projects; i++)
    {
        outP.write(QString::number(i).toUtf8());
        outP.write(" ");
        outP.write(QString::number(distSupervisors(gen)).toUtf8());
        outP.write("\n");
    }

    outP.close();


    QMessageBox msg;
    msg.setText("Completed.");
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle("SPASuite Dataset Generator");
    msg.setWindowIcon(QIcon("resource/win_icon.png"));
    msg.exec();

}

