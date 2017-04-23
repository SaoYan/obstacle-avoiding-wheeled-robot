#ifndef CHANGECOMMANDS_H
#define CHANGECOMMANDS_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class ChangeCommands;
}

class ChangeCommands : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeCommands(QWidget *parent = 0);
    ~ChangeCommands();

private:
    Ui::ChangeCommands *ui;

private:
    int flag_cancle;
    //指令集
    QString FORWARD;        //前
    QString LEFT;           //左
    QString RIGHT;          //右
    QString LEFT_FORWARD;   //左前
    QString RIGHT_FORWARD;  //右前
    QString STOP;           //停止

private:
    void closeEvent(QCloseEvent *event);

signals:
    SaveConfiguration(QString forward,
                      QString left,QString right,
                      QString left_forward,QString right_forward,
                      QString stop);
private slots:
    void on_pushButton_Save_clicked();
    void on_pushButton_Cancle_clicked();

};

#endif // CHANGECOMMANDS_H
