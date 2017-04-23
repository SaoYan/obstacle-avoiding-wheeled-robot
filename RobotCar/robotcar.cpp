#include "robotcar.h"
#include "ui_robotcar.h"

RobotCar::RobotCar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RobotCar)
{
    ui->setupUi(this);
    Init_RobotCar();
}

RobotCar::~RobotCar()
{
    delete ui;
}

///********** Customer Functions **********///
void RobotCar::Init_RobotCar()
{
    ///初始化界面外观
    ui->label_disp->setPixmap(QPixmap(":/myRes/Background.jpg"));

    ui->label_2->setPixmap(QPixmap(":/myRes/redcross.png"));
    ui->label_2->setScaledContents(true);

    ui->label_ROI_Color->setPixmap(QPixmap("sample_color.jpg"));
    ui->label_ROI_Color->setScaledContents(true);
    ui->label_ROI_White->setPixmap(QPixmap("sample_white.jpg"));;
    ui->label_ROI_White->setScaledContents(true);

    ui->label_centerX_2->setText("-1");
    ui->label_centerY_2->setText("-1");

    ///实例分配
    timer_vision= new QTimer(this);
    timer_cmd   = new QTimer(this);
    timer_state = new QTimer(this);
    change_commands = new ChangeCommands(this);
    CarCom = new QSerialPort(this);

    ///start timer
    timer_state->start(10);
    timer_vision->start(20);
    timer_cmd->start(10);

    ///state
    ///调试时候可能会改变的参数有：
    /// rangeThX rangeThY
    rangeThX = 100;
    rangeThY = 200;
    round = 0;
    //状态初始化
    state = STATIC;
    //显示当前状态
    ui->label_state_2->setText("0 static");
    //标识符初始化
    flag_run = false;
    flag_avoid = false;
    flag_visible = false;
    flag_reach = false;
    flag_deviation = 0;
    flag_X = 0;
    flag_Y = 0;

    ///vision
    ///调试时候可能会改变的参数有：
    /// element1  element1
    /// finder_th  minSaturation
    /// minValue minArea
    //Initial sample
    ROI_Color=cv::imread("sample_color.jpg");
    ROI_White=cv::imread("sample_white.jpg");
    //OpenCV Objects
    element1=cv::Mat(5,5,CV_8U,cv::Scalar(1));
    element2=cv::Mat(10,10,CV_8U,cv::Scalar(1));
    //Detect
    finder_th=0.05f;
    minSaturation=0;
    minValue= 200;
    minArea = 2000;
    criteria=cv::TermCriteria(cv::TermCriteria::MAX_ITER,10,0.1);
    flag_init=false;
    centerX = -1 ; centerY = -1;
    //Get Sample
    flag_sample=false;
    rubber = new QRubberBand(QRubberBand::Rectangle, ui->label_disp);
    if(ui->radioButton_color->isChecked())
    {
        ROI_Preview=ROI_Color;
    }
    else if(ui->radioButton_white->isChecked())
    {
        ROI_Preview=ROI_White;
    }

    ///commands
    //初始化指令集
    FORWARD      =0x02;
    LEFT         =0x04;
    RIGHT        =0x06;
    LEFT_FORWARD =0x01;
    RIGHT_FORWARD=0x03;
    STOP         =0x05;
    command = STOP;
    //显示当前指令集
    ui->label_FORWARD_2->setText(SetTextForDisp(FORWARD));
    ui->label_LEFT_2->setText(SetTextForDisp(LEFT));
    ui->label_RIGHT_2->setText(SetTextForDisp(RIGHT));
    ui->label_LEFT_FORWARD_2->setText(SetTextForDisp(LEFT_FORWARD));
    ui->label_RIGHT_FORWARD_2->setText(SetTextForDisp(RIGHT_FORWARD));
    ui->label_STOP_2->setText(SetTextForDisp(STOP));
    ui->label_command_2->setText("STOP");
    //当前串口信息
    PortInfoList=QSerialPortInfo::availablePorts();
    if(PortInfoList.count()==0)
    {
        ComName.insert(0,"none !!");
    }
    else
    {
        for(int n=0;n<PortInfoList.count();n++)
        {
            ComName.insert(n,PortInfoList.at(n).portName());
        }
    }
    ui->comboBox_Ports->insertItems(0,ComName);
    PortInfoList.clear();
    ComName.clear();

    ///连接信号与槽
    connect(this->change_commands,
            SIGNAL(SaveConfiguration(QString,QString,QString,QString,QString,QString)),
            this,
            SLOT(SwitchConfiguration(QString,QString,QString,QString,QString,QString)));
    connect(this->CarCom,
            SIGNAL(readyRead()),
            this,
            SLOT(ReceiveMsg()));
    connect(this->timer_state,
            SIGNAL(timeout()),
            this,
            SLOT(UpdateState()));
    connect(this->timer_cmd,
            SIGNAL(timeout()),
            this,
            SLOT(SendCommand()));
}

///state
bool RobotCar::StartRun()
{
    //Open Serial Port
    if(ui->comboBox_Ports->currentText()=="none !!")
    {
        QMessageBox::warning(this,
                             "Wrong",
                             "No Serial Port Available !",
                             QMessageBox::Ok);
        return false;
    }
    else
    {
        CarCom->setPortName(ui->comboBox_Ports->currentText());
        CarCom->open(QSerialPort::ReadWrite);
        CarCom->setBaudRate(QSerialPort::Baud9600); //波特率
        CarCom->setParity(QSerialPort::NoParity);   //无奇偶校验位
        CarCom->setDataBits(QSerialPort::Data8);    //八位数据
        CarCom->setStopBits(QSerialPort::OneStop);  //一位停止位
        CarCom->setFlowControl(QSerialPort::NoFlowControl);
        if(CarCom->isOpen())
        {
            //起始指令
            SetCommand(STOP);
            //切换外观
            ui->label_2->setPixmap(QPixmap(":/myRes/green.jpg"));
            ui->label_2->setScaledContents(true);
        }
        else
        {
            QMessageBox::warning(this,"Fail",
                                 "Fail Opening Serial Port !",
                                 QMessageBox::Ok);
            return false;
        }
    }

    //Open Camera
    if (!videoCapture.isOpened())
    {
        videoCapture.open(0);
        if(videoCapture.isOpened())
        {
            connect(this->timer_vision,SIGNAL(timeout()),this,SLOT(RealTimeCap()));
            connect(this->timer_vision,SIGNAL(timeout()),this,SLOT(Detect()));
            return true;
        }
        else
        {
           QMessageBox::warning(this,"Warning","Camera Open Failed!",QMessageBox::Ok);
           return false;
        }
    }

    return true;
}

void RobotCar::StopRun()
{
    //Send Command
    SetCommand(STOP);
    //Close Camera
    disconnect(this->timer_vision,SIGNAL(timeout()),this,SLOT(RealTimeCap()));
    disconnect(this->timer_vision,SIGNAL(timeout()),this,SLOT(Detect()));
    cv::destroyWindow("Find Target");
    cv::destroyWindow("Trace Target");
    videoCapture.release();
    //Reset Flag
    flag_run = false;
    flag_avoid = false;
    flag_visible = false;
    flag_reach = false;
    flag_deviation = 0;
    //Reset State
    state = STATIC;
    ui->label_state_2->setText("0 static");
    //更新外观
    ui->label_disp->setPixmap(QPixmap(":/myRes/Background.jpg"));
    ui->label_centerX_2->setText("-1");
    ui->label_centerY_2->setText("-1");
}

void RobotCar::RunToEnd()
{
    //更新flag_X
    if(centerX < image_detect.cols/2-rangeThX)
    {
        flag_X = 1; //左
    }
    else if(centerX > image_detect.cols/2+rangeThX)
    {
        flag_X = 2; //右
    }
    else
    {
        flag_X = 0;
    }
    //更新flag_Y
    if(centerY < image_detect.rows/2-rangeThY)
    {
        flag_Y = 1; //上
    }
    else if(centerY > image_detect.rows/2+rangeThY)
    {
        flag_Y = 2; //下
    }
    else
    {
        flag_Y = 0;
    }
    //执行策略，更新指令
    switch(flag_Y)
    {
    case 0:
    case 1:
        switch(flag_X)
        {
        case 0: SetCommand(FORWARD);break;
        case 1: SetCommand(LEFT_FORWARD);break;
        case 2: SetCommand(RIGHT_FORWARD);break;
        }
        break;
    case 2: //偏下；延时后停止
        cv::waitKey(100);
        flag_reach = true;
        break;
    }
}

///vision
void RobotCar::mousePressEvent(QMouseEvent *event)
{
    if (flag_sample)
    {
        startPosition = event ->pos();
        rubber->setGeometry(QRect(startPosition-ui->label_disp->pos(), QSize()));
        rubber->show();
    }
}

void RobotCar::mouseMoveEvent(QMouseEvent *event)
{
    if (flag_sample)
    {
        rubber->setGeometry(QRect(startPosition-ui->label_disp->pos(),
                                  event->pos()-ui->label_disp->pos()).normalized());
    }
}

void RobotCar::mouseReleaseEvent(QMouseEvent *event)
{
    if (flag_sample)
    {
        ///坐标处理
        endPosition = event->pos();
        //label在窗口中的坐标
        int  labelX = ui->label_disp->x();
        int  labelY = ui->label_disp->y();
        //鼠标起点和终点的坐标
        int sx = startPosition.x();
        int sy = startPosition.y();
        int ex = endPosition.x();
        int ey = endPosition.y();
        //坐标最值
        int minX = sx < ex ? sx : ex;     //sx，ex中较小的
        int maxX = sx + ex - minX;
        int minY = sy < ey ? sy : ey;     //sy，ey中较小的
        int maxY = sy + ey - minY;
        //异常情况处理:矩形框超出图像边缘
        if(minX < labelX) minX = labelX;
        if(minY < labelY) minY = labelY;
        if(maxX > labelX + image.cols) maxX = labelX + image.cols;
        if(maxY > labelY + image.rows) maxY = labelY + image.rows;
        //重新对点赋值
        //始终使startPosition是矩形框左上角，endPosition是矩形框右下角
        startPosition.setX(minX);
        startPosition.setY(minY);
        endPosition.setX(maxX);
        endPosition.setY(maxY);

        //鼠标圈出的区域
        selection=cv::Rect(startPosition.x()-ui->label_disp->x(),
                           startPosition.y()-ui->label_disp->y(),
                           endPosition.x()-startPosition.x(),
                           endPosition.y()-startPosition.y());
        temp=cv::Mat(image,selection);
        temp.copyTo(ROI_Preview);
        //如果不以temp为“中介”，会产生奇怪的后果

        rubber->hide();
    }
}

///commands
QString RobotCar::SetTextForDisp(QString str)
{
    uint value=str.toUInt(0,16);
    if(value<0x10)
    {
        return("0x0"+QString::number(value,16));
    }
    else
    {
        return("0x"+QString::number(value,16));
    }
}

QString RobotCar::SetTextForDisp(uint value)
{
    if(value<0x10)
    {
        return("0x0"+QString::number(value,16));
    }
    else
    {
        return("0x"+QString::number(value,16));
    }
}

void RobotCar::SetCommand(uint cmd)
{
    command = cmd ;
    //更新显示信息
    if(command==FORWARD)
    {
        ui->label_command_2->setText("FORWARD");
    }
    else if(command==LEFT)
    {
        ui->label_command_2->setText("LEFT");
    }
    else if(command==RIGHT)
    {
        ui->label_command_2->setText("RIGHT");
    }
    else if(command==LEFT_FORWARD)
    {
        ui->label_command_2->setText("LEFT_FORWARD");
    }
    else if(command==RIGHT_FORWARD)
    {
        ui->label_command_2->setText("RIGHT_FORWARD");
    }
    else if(command==STOP)
    {
        ui->label_command_2->setText("STOP");
    }
}

///********** Customer Slots **********///
/// state
void RobotCar::UpdateState()
{
    switch(state)
    {
    case STATIC    :
        ui->label_state_2->setText("0 static");
        if(flag_run)
        {
            state = AVOIDANCE;
        }
        else
        {
            state = STATIC;
        }
        break;
    case AVOIDANCE :
        ui->label_state_2->setText("1 avoidance");
        if(flag_avoid)
        {
            if(flag_deviation==1)
            {
                SetCommand(RIGHT);
                state = CORRECTION;
            }
            else if(flag_deviation==2)
            {
                SetCommand(LEFT);
                state = CORRECTION;
            }
            else
            {
                state = AVOIDANCE;
            }
        }
        else
        {
            state = AVOIDANCE;
        }
        break;
    case CORRECTION:
        ui->label_state_2->setText("2 correction");
        if(flag_visible)
        {
            state = DETECTION;
            round = 0;
        }
        else
        {
            state = CORRECTION;
        }

        if (round == 30) //平移仍然找不到色标，可能是下位机发错信息了
        {
            if (command == RIGHT)
            {
                SetCommand(LEFT);
            }
            else
            {
                SetCommand(RIGHT);
            }
            state = CORRECTION;
        }
        round++;
        break;
    case DETECTION :
        ui->label_state_2->setText("3 detection");
        if(centerX == -1 || centerY == -1) //跑偏了，需要返回 CORRECTION 状态
        {
            flag_visible = false;
            round = 0;
            if(flag_deviation==1)
            {
                SetCommand(RIGHT);
                state = CORRECTION;
            }
            else if(flag_deviation==2)
            {
                SetCommand(LEFT);
                state = CORRECTION;
            }
            else
            {
                state = AVOIDANCE;
            }
        }
        else
        {
            if(flag_reach)
            {
                StopRun();
                state = STATIC;
            }
            else
            {
                //执行算法
                RunToEnd();
                state = DETECTION;
            }
        }
        break;
    }
}

///vision
void RobotCar::RealTimeCap()
{
    videoCapture >> image;

    cv::cvtColor(image,image_rgb,CV_BGR2RGB);
    img = QImage((const unsigned char*)image_rgb.data,
                        image_rgb.cols, image_rgb.rows,
                        QImage::Format_RGB888);
    ui->label_disp->setPixmap(QPixmap::fromImage(img));
}

void RobotCar::Detect()
{
    //识别彩色
    if (ui->radioButton_color->isChecked())
    {
        ColorHistogram hc;
        ContentFinder finder;
        image_detect=image.clone();
        ///使用image的一个拷贝作为下面处理的对象，而不是videoCapture >> image_hist;
        ///这是因为如果使用后者，image_hist和image实际占用同一段内存
        ///处理image_hist也会造成image的改变，产生不符合预期的结果
        //获取ROI直方图
        hc.getHueHistogram(ROI_Color,minSaturation);
        //识别当前图像低饱和度像素
        cv::cvtColor(image_detect,image_detect,CV_BGR2HSV);
        cv::split(image_detect,v);  //v[1]是饱和度分量
        cv::threshold(v[1],v[1],minSaturation,255,cv::THRESH_BINARY);
        //寻找目标
        finder.setHistogram(hc.hist);
        finder.setThreshold(finder_th);
        finder.findHueContent(image_detect,0.0f,180.0f,1);
        //去除低饱和度像素
        cv::bitwise_and(finder.result,v[1],finder.result);
        //腐蚀膨胀，去除孔洞 & 连接区域间缝隙
        cv::erode(finder.result,finder.result,cv::Mat());
        cv::dilate(finder.result,finder.result,element2);
        //直方图反投影方法初始化矩形框
        if (!flag_init)
        {
            //提取连通区域轮廓
            image_detect=finder.result.clone();
            ///直接把finder.result送到findContours函数中会导致崩溃
            ///可能原因是findContours直接操作finder的私有成员导致错误
            cv::findContours(image_detect,contours,
                         CV_RETR_EXTERNAL,
                         CV_CHAIN_APPROX_NONE);
            //提取最大包围盒
            boundRect=std::vector<cv::Rect>(contours.size());
            for(uint i=0;i<contours.size();i++)
            {
                boundRect[i]=cv::boundingRect(cv::Mat(contours[i]));
            }
            //寻找最大包围盒
            if(contours.size()!=0)
            {
                for(uint i=0,pos_max=0; i<contours.size(); i++)
                {
                    if(boundRect[pos_max].area() <= boundRect[i].area())
                    {
                        pos_max=i;
                        rect=boundRect[pos_max];
                    }
                }
            }

            flag_init=true;
        }
        //CamShift 算法
        if (rect.area()>minArea)
        {
            image_detect=finder.result.clone();
            cv::CamShift(image_detect,rect,criteria);
        }
        else
        {
            flag_init=false;
        }

        //
        image_detect=image.clone();
        if (rect.area()>minArea)
        {
            cv::rectangle(image_detect,rect,cv::Scalar(0,255,0),2); //绿色矩形框
            cv::circle(image_detect,
                       cv::Point(rect.x+rect.width/2,rect.y+rect.height/2),
                       5,cv::Scalar(255,0,0),2);
            //更新参数
            centerX = rect.x+rect.width/2;
            centerY = rect.y+rect.height/2;
        }
        else
        {
            cv::rectangle(image_detect,rect,cv::Scalar(0,0,255),2);  //红色矩形框
            cv::circle(image_detect,
                       cv::Point(rect.x+rect.width/2,rect.y+rect.height/2),
                       5,cv::Scalar(255,0,0),2);
            centerX = -1;
            centerY = -1;
        }

        //更新标识符
        if(!flag_init)
        {
            flag_visible = false;
        }
        else
        {
            flag_visible = true;
        }
        //实时显示
        cv::namedWindow("Find Target");
        cv::imshow("Find Target",finder.result);
        cv::namedWindow("Trace Target");
        cv::imshow("Trace Target",image_detect);
        ui->label_centerX_2->setText(QString::number(centerX));
        ui->label_centerY_2->setText(QString::number(centerY));
    }
    //识别白色
    else if (ui->radioButton_white->isChecked())
    {
        ///和识别彩色有两点不同，其余都一样
        /// 1. 获取ROI的饱和度直方图
        /// 2. 去除低明度像素
        ColorHistogram hc;
        ContentFinder finder;
        image_detect=image.clone();
        //获取ROI直方图
        hc.getSatHistogram(ROI_White,minValue);
        //识别当前图像低明度像素
        cv::cvtColor(image_detect,image_detect,CV_BGR2HSV);
        cv::split(image_detect,v);  //v[2]是色度分量
        cv::threshold(v[2],v[2],minValue,255,cv::THRESH_BINARY);
        //寻找目标
        finder.setHistogram(hc.hist);
        finder.setThreshold(finder_th);
        finder.findSatContent(image_detect,0.0f,180.0f);
        //去除低饱和度像素
        cv::bitwise_and(finder.result,v[2],finder.result);
        //腐蚀膨胀，去除孔洞 & 连接区域间缝隙
        cv::erode(finder.result,finder.result,cv::Mat());
        cv::dilate(finder.result,finder.result,element2);
        //直方图反投影方法初始化矩形框
        if (!flag_init)
        {
            //提取连通区域轮廓
            image_detect=finder.result.clone();
            cv::findContours(image_detect,contours,
                         CV_RETR_EXTERNAL,
                         CV_CHAIN_APPROX_NONE);
            //提取最大包围盒
            boundRect=std::vector<cv::Rect>(contours.size());
            for(uint i=0;i<contours.size();i++)
            {
                boundRect[i]=cv::boundingRect(cv::Mat(contours[i]));
            }
            //寻找最大包围盒
            if(contours.size()!=0)
            {
                for(uint i=0,pos_max=0; i<contours.size(); i++)
                {
                    if(boundRect[pos_max].area() <= boundRect[i].area())
                    {
                        pos_max=i;
                        rect=boundRect[pos_max];
                    }
                }
            }

            flag_init=true;
        }
        //CamShift 算法
        if (rect.area()>minArea)
        {
            image_detect=finder.result.clone();
            cv::CamShift(image_detect,rect,criteria);
        }
        else
        {
            flag_init=false;
        }

        //
        image_detect=image.clone();
        if (rect.area()>minArea)
        {
            cv::rectangle(image_detect,rect,cv::Scalar(0,255,0),2); //绿色矩形框
            cv::circle(image_detect,
                       cv::Point(rect.x+rect.width/2,rect.y+rect.height/2),
                       5,cv::Scalar(255,0,0),2);
            //更新参数
            centerX = rect.x+rect.width/2;
            centerY = rect.y+rect.height/2;
        }
        else
        {
            cv::rectangle(image_detect,rect,cv::Scalar(0,0,255),2);  //红色矩形框
            cv::circle(image_detect,
                       cv::Point(rect.x+rect.width/2,rect.y+rect.height/2),
                       5,cv::Scalar(255,0,0),2);
            centerX = -1;
            centerY = -1;
        }

        //更新标识符
        if(!flag_init)
        {
            flag_visible = false;
        }
        else
        {
            flag_visible = true;
        }
        //实时显示
        cv::namedWindow("Find Target");
        cv::imshow("Find Target",finder.result);
        cv::namedWindow("Trace Target");
        cv::imshow("Trace Target",image_detect);
        ui->label_centerX_2->setText(QString::number(centerX));
        ui->label_centerY_2->setText(QString::number(centerY));
    }
}

void RobotCar::GetSample()
{
    ///采集色标时候的预览效果应该与实际效果相同
    /// 因此这部分图像处理算法应该与 Detect 完全相同
    /// （当然部分变量名字不同）
    //识别彩色
    if (ui->radioButton_color->isChecked())
    {
        ColorHistogram hc;
        ContentFinder finder;
        image_sample=image.clone();
        //获取ROI直方图
        hc.getHueHistogram(ROI_Preview,minSaturation);
        //识别当前图像低饱和度像素
        cv::cvtColor(image_sample,image_sample,CV_BGR2HSV);
        cv::split(image_sample,v);  //v[1]是饱和度分量
        cv::threshold(v[1],v[1],minSaturation,255,cv::THRESH_BINARY);
        //寻找目标
        finder.setHistogram(hc.hist);
        finder.setThreshold(finder_th);
        finder.findHueContent(image_sample,0.0f,180.0f,1);
        //去除低饱和度像素
        cv::bitwise_and(finder.result,v[1],finder.result);
        //腐蚀膨胀，去除孔洞 & 连接区域间缝隙
        cv::erode(finder.result,finder.result,cv::Mat());
        cv::dilate(finder.result,finder.result,element2);
        //直方图反投影方法初始化矩形框
        if (!flag_init)
        {
            //提取连通区域轮廓
            image_sample=finder.result.clone();
            cv::findContours(image_sample,contours,
                         CV_RETR_EXTERNAL,
                         CV_CHAIN_APPROX_NONE);
            //提取最大包围盒
            boundRect=std::vector<cv::Rect>(contours.size());
            for(uint i=0;i<contours.size();i++)
            {
                boundRect[i]=cv::boundingRect(cv::Mat(contours[i]));
            }
            //寻找最大包围盒
            if(contours.size()!=0)
            {
                for(uint i=0,pos_max=0; i<contours.size(); i++)
                {
                    if(boundRect[pos_max].area() <= boundRect[i].area())
                    {
                        pos_max=i;
                        rect=boundRect[pos_max];
                    }
                }
            }
            flag_init=true;
        }
        //CamShift 算法
        if (rect.area()>0)
        {
            image_sample=finder.result.clone();
            cv::CamShift(image_sample,rect,criteria);
        }
        else
        {
            flag_init=false;
        }

        image_sample=image.clone();
        if (rect.area()>0)
        {
            cv::rectangle(image_sample,rect,cv::Scalar(0,255,0),2);
            cv::circle(image_sample,
                       cv::Point(rect.x+rect.width/2,rect.y+rect.height/2),
                       5,cv::Scalar(255,0,0),2);
        }
        cv::namedWindow("Preview 01");
        cv::imshow("Preview 01",finder.result);
        cv::namedWindow("Preview 02");
        cv::imshow("Preview 02",image_sample);
    }
    //识别白色
    else if (ui->radioButton_white->isChecked())
    {
        ColorHistogram hc;
        ContentFinder finder;
        image_sample=image.clone();
        //获取ROI直方图
        hc.getSatHistogram(ROI_Preview,minValue);
        //识别当前图像低明度像素
        cv::cvtColor(image_sample,image_sample,CV_BGR2HSV);
        cv::split(image_sample,v);  //v[2]是色度分量
        cv::threshold(v[2],v[2],minValue,255,cv::THRESH_BINARY);
        //寻找目标
        finder.setHistogram(hc.hist);
        finder.setThreshold(finder_th);
        finder.findSatContent(image_sample,0.0f,180.0f);
        //去除低饱和度像素
        cv::bitwise_and(finder.result,v[2],finder.result);
        //腐蚀膨胀，去除孔洞 & 连接区域间缝隙
        cv::erode(finder.result,finder.result,cv::Mat());
        cv::dilate(finder.result,finder.result,element2);
        //直方图反投影方法初始化矩形框
        if (!flag_init)
        {
            //提取连通区域轮廓
            image_sample=finder.result.clone();
            cv::findContours(image_sample,contours,
                         CV_RETR_EXTERNAL,
                         CV_CHAIN_APPROX_NONE);
            //提取最大包围盒
            boundRect=std::vector<cv::Rect>(contours.size());
            for(uint i=0;i<contours.size();i++)
            {
                boundRect[i]=cv::boundingRect(cv::Mat(contours[i]));
            }
            //寻找最大包围盒
            if(contours.size()!=0)
            {
                for(uint i=0,pos_max=0; i<contours.size(); i++)
                {
                    if(boundRect[pos_max].area() <= boundRect[i].area())
                    {
                        pos_max=i;
                        rect=boundRect[pos_max];
                    }
                }
            }

            flag_init=true;
        }
        //CamShift 算法
        if (rect.area()>0)
        {
            image_sample=finder.result.clone();
            cv::CamShift(image_sample,rect,criteria);
        }
        else
        {
            flag_init=false;
        }

        image_sample=image.clone();
        if (rect.area()>0)
        {
            cv::rectangle(image_sample,rect,cv::Scalar(0,255,0),2);
            cv::circle(image_sample,
                       cv::Point(rect.x+rect.width/2,rect.y+rect.height/2),
                       5,cv::Scalar(255,0,0),2);
        }
        cv::namedWindow("Preview 01");
        cv::imshow("Preview 01",finder.result);
        cv::namedWindow("Preview 02");
        cv::imshow("Preview 02",image_sample);
    }
}

///commands
void RobotCar::SwitchConfiguration(QString forward,
                         QString left,QString right,
                         QString left_forward,QString right_forward,
                         QString stop)
{
    if(forward!="0x00")
    {
        FORWARD=forward.toUInt(0,16);
        ui->label_FORWARD_2->setText(SetTextForDisp(forward));
    }
    if(left!="0x00")
    {
        LEFT=left.toUInt(0,16);
        ui->label_LEFT_2->setText(SetTextForDisp(left));
    }
    if(right!="0x00")
    {
        RIGHT=right.toUInt(0,16);
        ui->label_RIGHT_2->setText(SetTextForDisp(right));
    }
    if(left_forward!="0x00")
    {
        LEFT_FORWARD=left_forward.toUInt(0,16);
        ui->label_LEFT_FORWARD_2->setText(SetTextForDisp(left_forward));
    }
    if(right_forward!="0x00")
    {
        RIGHT_FORWARD=right_forward.toUInt(0,16);
        ui->label_RIGHT_FORWARD_2->setText(SetTextForDisp(right_forward));
    }
    if(stop!="0x00")
    {
        STOP=stop.toUInt(0,16);
        ui->label_STOP_2->setText(SetTextForDisp(stop));
    }
}

void RobotCar::SendCommand()
{
    if(CarCom->isOpen())
    {
        if(ui->checkBox->isChecked())
        {
            MesgSend.append(0xfa);
            MesgSend.append(0xaf);
        }
        MesgSend.append(command);
        CarCom->write(MesgSend);
        MesgSend.clear();
    }
}

void RobotCar::ReceiveMsg()
{
    MesgReceive = CarCom->readAll();

    ui->label_Compass->setText(MesgReceive.toHex());
    qDebug() << MesgReceive.size();

    for(int i=0 ; i<MesgReceive.size() ; i++)
    {
        switch (BuffCar.size())
        {
        case 0:
            if((uchar)MesgReceive.at(i)==(uchar)0xfa)
            {
                BuffCar.append(MesgReceive.at(i));
            }
            break;
        case 1:
            BuffCar.append(MesgReceive.at(i));
            break;
        case 2:
            BuffCar.append(MesgReceive.at(i));
            //数据处理
            //数据1：是否完成壁障
            if((uchar)BuffCar.at(1)==(uchar)0x00)
            {
                flag_avoid = false;
            }
            else if((uchar)BuffCar.at(1)==(uchar)0x08)
            {
                flag_avoid = true;
            }
            else
            {
                flag_avoid = false;
            }
            //数据2：航向偏离
            if((uchar)BuffCar.at(2)==(uchar)0x07)
            {
                flag_deviation = 1;
            }
            else if((uchar)BuffCar.at(2)==(uchar)0x09)
            {
                flag_deviation = 2;
            }
            else
            {
                flag_deviation = 0;
            }
            //处理完毕
            BuffCar.clear();
            break;
        default:
            BuffCar.clear();
        }
    }
    MesgReceive.clear();
}

///********** System Slots **********///
void RobotCar::on_pushButton_Start_clicked()
{
    ///如果任意一个初始化步骤出错
    /// 系统都不能正确启动
    if(!StartRun())
    {
        CarCom->close();

        disconnect(this->timer_vision,SIGNAL(timeout()),this,SLOT(RealTimeCap()));
        disconnect(this->timer_vision,SIGNAL(timeout()),this,SLOT(Detect()));

        ui->label_2->setPixmap(QPixmap(":/myRes/redcross.png"));
        ui->label_2->setScaledContents(true);
        ui->label_disp->setPixmap(QPixmap(":/myRes/Background.jpg"));

        flag_run = false;
    }
}

void RobotCar::on_pushButton_Run_clicked()
{
    if (!CarCom->isOpen())  //根据串口是否打开判断是否已经初始化（点过了“start”按钮）
    {
        QMessageBox::warning(this,"Wrong","Please Start First!",
                             QMessageBox::Ok);
    }
    else
    {
        SetCommand(FORWARD);
        flag_run = true;
    }
}

void RobotCar::on_pushButton_Stop_clicked()
{
    StopRun();
}

void RobotCar::on_pushButton_close_clicked()
{
    //Close Port
    CarCom->close();
    ui->label_2->setPixmap(QPixmap(":/myRes/redcross.png"));
    ui->label_2->setScaledContents(true);
}

void RobotCar::on_pushButton_Change_Commands_clicked()
{
    change_commands->show();
}

void RobotCar::on_pushButton_Get_Sample_clicked()
{
    if(videoCapture.isOpened())
    {
        QMessageBox::information(this,"Instruction",
                                 "在图像中框出色标！  点击Ok开始采集色标",
                                 QMessageBox::Ok);
        cv::namedWindow("Preview 01");
        cv::namedWindow("Preview 02");
        flag_sample=true;
        connect(this->timer_vision,SIGNAL(timeout()),this,SLOT(GetSample()));
        //防止冲突
        disconnect(this->timer_vision,SIGNAL(timeout()),this,SLOT(CalcBackProject()));
        disconnect(this->timer_vision,SIGNAL(timeout()),this,SLOT(Detect()));
        cv::destroyWindow("Find Target");
        cv::destroyWindow("Trace Target");
    }
    else
    {
        QMessageBox::warning(this,"Warning",
                             "Open Camera First!",
                             QMessageBox::Ok);
    }
}

void RobotCar::on_pushButton_Save_Sample_clicked()
{
    if (flag_sample)
    {
        QMessageBox mb(tr("Save"), tr("是否更新色标信息？"),
                       QMessageBox::Question,
                       QMessageBox::Yes | QMessageBox::Default,
                       QMessageBox::No | QMessageBox::Escape,
                       QMessageBox::NoButton);
        if(mb.exec() == QMessageBox::Yes)
        {
            disconnect(this->timer_vision,SIGNAL(timeout()),this,SLOT(GetSample()));
            connect(this->timer_vision,SIGNAL(timeout()),this,SLOT(RealTimeCap()));
            connect(this->timer_vision,SIGNAL(timeout()),this,SLOT(Detect()));
            cv::destroyWindow("Preview 01");
            cv::destroyWindow("Preview 02");
            if(ui->radioButton_color->isChecked())
            {
                ROI_Color=ROI_Preview;
                cv::imwrite("sample_color.jpg",ROI_Color);
                ui->label_ROI_Color->setPixmap(QPixmap("sample_color.jpg"));
                ui->label_ROI_Color->setScaledContents(true);
            }
            else if(ui->radioButton_white->isChecked())
            {
                ROI_White=ROI_Preview;
                cv::imwrite("sample_white.jpg",ROI_White);
                ui->label_ROI_White->setPixmap(QPixmap("sample_white.jpg"));
                ui->label_ROI_White->setScaledContents(true);
            }
        }
        else
        {
            disconnect(this->timer_vision,SIGNAL(timeout()),this,SLOT(GetSample()));
            connect(this->timer_vision,SIGNAL(timeout()),this,SLOT(RealTimeCap()));
            connect(this->timer_vision,SIGNAL(timeout()),this,SLOT(Detect()));
            cv::destroyWindow("Preview 01");
            cv::destroyWindow("Preview 02");
        }
        flag_sample = false;
    }
    else
    {
        QMessageBox::warning(this,"Warning",
                             "Please Get Sample First!",
                             QMessageBox::Ok);
    }
}

void RobotCar::on_pushButton_refresh_clicked()
{
    ui->comboBox_Ports->clear();
    PortInfoList=QSerialPortInfo::availablePorts();
    if(PortInfoList.count()==0)
    {
        ComName.insert(0,"none !!");
    }
    else
    {
        for(int n=0;n<PortInfoList.count();n++)
        {
            ComName.insert(n,PortInfoList.at(n).portName());
        }
    }
    ui->comboBox_Ports->insertItems(0,ComName);
    PortInfoList.clear();
    ComName.clear();
}
