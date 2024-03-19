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
    std::uniform_int_distribution<> distCapacities(1, ui->maxCapacityVal->value());

    // Select save location
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Dataset"),
                                                    "dataset.txt",
                                                    tr("Text Files (*.txt)"));

    QFile out(fileName);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    out.write(QString::number(students).toUtf8());
    out.write(" ");
    out.write(QString::number(supervisors).toUtf8());
    out.write(" ");
    out.write(QString::number(projects).toUtf8());
    out.write("\n");

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

        out.write(QString::number(i+1).toUtf8());
        for (int j = 0; j < projects; j++)
        {
            if (distReals(gen) >= incomplete) // skip assignment if lower than incomplete rate
            {
                out.write(" ");
                out.write(QString::number(tempProjects[j]+1).toUtf8());
                out.write(":");
                out.write(QString::number(pref).toUtf8());

                if (distReals(gen) >= ties)
                {
                    pref++;
                }
            }
        }
        out.write("\n");
        pref = 1;
    }

    // Build supervisors
    for (int i = 0; i < supervisors; i++)
    {
        std::random_shuffle(tempStudents.begin(), tempStudents.end());

        out.write(QString::number(i+1).toUtf8());
        out.write(":");
        out.write(QString::number(distCapacities(gen)).toUtf8());
        for (int j = 0; j < students; j++)
        {
            if (distReals(gen) >= incomplete) // skip assignment if lower than incomplete rate
            {
                out.write(" ");
                out.write(QString::number(tempStudents[j]+1).toUtf8());
                out.write(":");
                out.write(QString::number(pref).toUtf8());

                if (distReals(gen) >= ties)
                {
                    pref++;
                }
            }
        }
        out.write("\n");
        pref = 1;
    }

    // Build projects
    for (int i = 0; i < projects; i++)
    {
        out.write(QString::number(i+1).toUtf8());
        out.write(":");
        out.write(QString::number(distCapacities(gen)).toUtf8());
        out.write(" ");
        out.write(QString::number(distSupervisors(gen)+1).toUtf8());
        out.write("\n");
    }

    out.close();


    QMessageBox msg;
    msg.setText("Completed.");
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle("SPASuite Dataset Generator");
    msg.setWindowIcon(QIcon("resource/win_icon.png"));
    msg.exec();

}

