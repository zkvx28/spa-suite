#ifndef DATAGENWINDOW_H
#define DATAGENWINDOW_H

#include <QDialog>

namespace Ui {
class DataGenWindow;
}

class DataGenWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DataGenWindow(QWidget *parent = nullptr);
    ~DataGenWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::DataGenWindow *ui;
};

#endif // DATAGENWINDOW_H
