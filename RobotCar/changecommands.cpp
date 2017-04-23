#include "changecommands.h"
#include "ui_changecommands.h"

ChangeCommands::ChangeCommands(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeCommands)
{
    ui->setupUi(this);
    flag_cancle=0;
}

ChangeCommands::~ChangeCommands()
{
    delete ui;
}

void ChangeCommands::closeEvent(QCloseEvent *event)
{
    if(!flag_cancle)
    {
        QMessageBox mb(tr("Save"), tr("是否更新指令配置信息？"),
                       QMessageBox::Question,
                       QMessageBox::Yes | QMessageBox::Default,
                       QMessageBox::No | QMessageBox::Escape,
                       QMessageBox::NoButton);
        if(mb.exec() == QMessageBox::Yes)
        {
            FORWARD      =ui->lineEdit_FORWARD->text().isEmpty() ?
                           "0x00" : ui->lineEdit_FORWARD->text();
            LEFT         =ui->lineEdit_LEFT->text().isEmpty() ?
                           "0x00" : ui->lineEdit_LEFT->text();
            RIGHT        =ui->lineEdit_RIGHT->text().isEmpty() ?
                           "0x00" : ui->lineEdit_RIGHT->text();
            LEFT_FORWARD = ui->lineEdit_LEFT_FORWARD->text().isEmpty() ?
                            "0x00" : ui->lineEdit_LEFT_FORWARD->text();
            RIGHT_FORWARD=ui->lineEdit_RIGHT_FORWARD->text().isEmpty() ?
                           "0x00" : ui->lineEdit_RIGHT_FORWARD->text();
            STOP         =ui->lineEdit_STOP->text().isEmpty()?
                        "0x00" : ui->lineEdit_STOP->text();
            emit SaveConfiguration(FORWARD,
                                   LEFT,RIGHT,
                                   LEFT_FORWARD,RIGHT_FORWARD,
                                   STOP);
        }
    }
    else
    {
        flag_cancle=0;
    }
}

void ChangeCommands::on_pushButton_Save_clicked()
{
    this->close(); //注意这会触发closeEvent事件
}

void ChangeCommands::on_pushButton_Cancle_clicked()
{
    flag_cancle=1;
    this->close();
}
