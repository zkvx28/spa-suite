#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();

    void on_actionDataset_Generator_triggered();

    void on_cooperStartButton_clicked();

    int getAssignableStudent(QList<int>, QList<int>);

    void on_loadDatasetButton_clicked();

    bool validateDataset(QString datasetPath);

    void disableAllControls();

    void enableAllControls();

private:
    void freeAndClearSupervisors();
    void freeAndClearStudents();
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
