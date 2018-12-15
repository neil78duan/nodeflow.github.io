#ifndef LINEINPUTDIALOG_H
#define LINEINPUTDIALOG_H



#include <QDialog>
#include <QString>

namespace Ui {
class LineInputDialog;
}

class LineInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LineInputDialog(QWidget *parent = 0);
    ~LineInputDialog();
    QString getInput() ;

private slots:
    void on_pushButton_clicked();

private:
    Ui::LineInputDialog *ui;
};

bool bp_init_console_input(QWidget *parent);
QString bp_console_input();

#endif // LINEINPUTDIALOG_H
