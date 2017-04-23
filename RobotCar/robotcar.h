#ifndef ROBOTCAR_H
#define ROBOTCAR_H

//QT
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QVector>
#include <QDebug>
#include <QPixmap>
#include <QImage>
#include <QPoint>
#include <QMoveEvent>
#include <QRubberBand>
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

//Customer
#include "colorhistogram.h"
#include "contentfinder.h"
#include "changecommands.h"

namespace Ui {
class RobotCar;
}

class RobotCar : public QWidget
{
    Q_OBJECT

public:
    explicit RobotCar(QWidget *parent = 0);
    ~RobotCar();

private:
    Ui::RobotCar *ui;
    ///state & strategy
    /// 0  停滞状态
    /// 1  壁障；初始化工作，不执行任何算法和策略
    /// 2  壁障结束后视野中无色标；车左（右）水平移动直到看到色标
    /// 3  追踪色标；
    int state;
    int round;
    QTimer *timer_state;
    int rangeThX;  //判断色标偏离的X方向阈值
    int rangeThY;  //判断色标偏离的Y方向阈值
    //状态定义
    #define STATIC     0
    #define AVOIDANCE  1
    #define CORRECTION 2
    #define DETECTION  3
    //标识符
    bool flag_run;       //机器人是否已正常启动
    bool flag_avoid;     //是否已经完成壁障
    bool flag_visible;   //终点线当前是否在视野内
    bool flag_reach;     //是否已经到达终点线
    int  flag_deviation; //车偏离航线方向（1：左偏；2：右偏；0：无效）

    int flag_X;          //detect 色标X方向偏离（1：左偏；2：右偏；0：无效）
    int flag_Y;          //detect 色标Y方向偏离（1：上偏；2：下偏；0：无效）

    ///vision
    //QT
    QTimer *timer_vision;
    QImage img;
    //OpenCV Objects
    cv::VideoCapture videoCapture;
    cv::Mat image;
    cv::Mat image_rgb;         //在label控件显示用
    cv::Mat ROI_Color;         //彩色色标
    cv::Mat ROI_White;         //白色色标
    cv::Mat ROI_Preview;       //取色标过程中临时存储
    cv::Mat element1,element2; //形态学运算模板
    //Get Sample
    bool flag_sample;          //标识符：是否开启取色标模式
    QPoint startPosition;      //鼠标起始点
    QPoint endPosition;        //鼠标终止点
    QRubberBand *rubber;       //橡皮筋线对象
    cv::Mat image_sample;      //在取色标过程中使用的临时图像
    cv::Rect selection;        //鼠标选取的区域
    cv::Mat temp;              //临时变量
    //Detect
    cv::Mat image_detect;       //在Detect程序部分使用的临时图像
    double finder_th;          //反投影检测的阈值
    int minSaturation;         //低饱和度像素阈值
    int minValue;              //低明度像素阈值
    int minArea;               //目标矩形框最小面积阈值
    cv::TermCriteria criteria; //算法参数定制
    bool flag_init;            //是否已经初始化过矩形框
    cv::vector <cv::Mat> v;    //存储H、S、V三通道
    int centerX , centerY;     //目标中心坐标（在图像坐标系中）
    //Contours
    cv::Rect rect;                                       //临时存储矩形框
    std::vector< std::vector<cv::Point> > contours;      //存储轮廓的数组
    std::vector< std::vector<cv::Point> >::iterator itc; //contours的迭代器
    std::vector<cv::Rect> boundRect;                     //存储包围盒的数组

    ///commands
    //Dialog
    ChangeCommands *change_commands;
    //timer
    QTimer *timer_cmd ;
    //SerialPort
    QList<QSerialPortInfo> PortInfoList;//当前可用串口信息
    QList<QString> ComName;    //当前可用串口名
    QSerialPort *CarCom;       //串口类
    QByteArray MesgSend;       //发送信息
    QByteArray MesgReceive;    //接收信息
    QByteArray BuffCar;        //接收数据缓存区
    //指令集
    uint FORWARD;        //前
    uint LEFT;           //左
    uint RIGHT;          //右
    uint LEFT_FORWARD;   //左前
    uint RIGHT_FORWARD;  //右前
    uint STOP;           //停止
    uint command ;       //当前指令

///*****Customer Functions*****///
private:
    void Init_RobotCar();
    ///state
    bool StartRun();
    void StopRun();
    void RunToEnd();
    ///vision
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    ///commands
    QString SetTextForDisp(QString str);
    QString SetTextForDisp(uint value);
    void SetCommand(uint cmd);

///*****Customer Slots*****///
private slots:
    ///state & strategy
    void UpdateState();
    ///vision
    void RealTimeCap();
    void Detect();
    void GetSample();
    ///commands
    void SwitchConfiguration(QString forward,
                             QString left,QString right,
                             QString left_forward,QString right_forward,
                             QString stop);
    void SendCommand();
    void ReceiveMsg();

///*****System Slots*****///
private slots:
    void on_pushButton_Start_clicked();

    void on_pushButton_Run_clicked();

    void on_pushButton_Stop_clicked();

    void on_pushButton_close_clicked();

    void on_pushButton_Change_Commands_clicked();

    void on_pushButton_Get_Sample_clicked();

    void on_pushButton_Save_Sample_clicked();

    void on_pushButton_refresh_clicked();

};

#endif // ROBOTCAR_H
