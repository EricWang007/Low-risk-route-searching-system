#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdlib.h>
#include <QPainter>
#include <string.h>
#include <QDebug>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include "algorithm.h"

#define Width 1220 //窗口宽度
#define Height 750 //窗口高度
#define width 400
#define height 750

//全局变量
static int anHour = 1000; //系统中一小时对应现实中多少毫秒
static int startcity, arrivecity;//起始城市和目的城市的序号
static int starttime, startday;//行程开始时间和开始天数
static int len = 0;//临时值，表示当前运行到行程的第几号班次
static int time0 = 0, day = 1;//系统当前时间和天数
static int success;//是否找到

//系统当前运行状态
//初始=-1，选完出发地=-2，选完目的地=-3，选完旅行策略=-4，未找到路线=-5
//=0表示在城市等待，=1表示在乘坐交通工具，=2表示到达一个城市
static int state = -1;
static int stop_flag = 0;//=1表示处于暂停状态，=0表示未处于暂停状态

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //设置窗口界面
    ui->setupUi(this);
    resize(Width+width, Height);
    this->setWindowTitle("covid-19低风险旅行查询系统");
    this->setWindowIcon(QIcon(":/soucre/plane.jpg"));
    ui->lineEdit_2->setReadOnly(true);

    //初始化，读入信息
    Init();

    //设置按钮颜色
    ui->pushButton_1->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_2->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_3->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_4->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_5->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_6->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_7->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_8->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_9->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_10->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_n->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_y->setStyleSheet("background-color: rgb(230,230,250)");
    ui->pushButton_r->setStyleSheet("background-color: rgb(230,230,250)");
    ui->slow->setStyleSheet("background-color: rgb(230,230,250)");
    ui->quick->setStyleSheet("background-color: rgb(230,230,250)");
    ui->enter->setStyleSheet("background-color: rgb(230,230,250)");
    ui->stop->setStyleSheet("background-color: rgb(230,230,250)");

    //连接城市按钮
    connectBut();

    //启动定时器，连接定时器和循环函数
    QTimer * timer = new QTimer(this);
    timer->start(anHour);
    connect(timer, &QTimer::timeout, [=](){
       timer->stop();
       timer->start(anHour);
       if(state == -2)
           timer->stop();
       if(state != -2)
           time0 = (time0 + 1) % 24;
       if(state >= 0)
           state = PassengerState(state, time0, &len, day);
       repaint();
       if(state == 2)
       {
           len++;
           state = 0;
       }
       if(len >= TotalLength && state >= 0)
       {
           repaint();
           qDebug()<<"end";
           timer->stop();
           printf("\nTotal risk: %.2f", TotalRisk);
           printf("\nTotal time: %dh\n", TotalTime);
           fprintf(fp3, "\nTotal risk: %.2f", TotalRisk);
           fprintf(fp3, "\nTotal time: %dh\n", TotalTime);
           fclose(fp3);
       }
       if(time0 == 0 && state >= 0)
           day++;
    });

    //连接旅行策略按钮及确认按钮，用于选择旅行策略和输入时限
    connect(ui->pushButton_n, &QPushButton::pressed, [=](){
        if(state == -3)
        {
            timelimit = nan;
            startDfs();
            repaint();
            state = PassengerState(state, time0, &len, day);
            repaint();
            timer->start(anHour);
        }
    });
    connect(ui->pushButton_y, &QPushButton::pressed, [=](){
        if(state == -3)
        {
            state = -4;
            repaint();
        }
    });
    connect(ui->enter, &QPushButton::pressed, [=](){
        QString tem;
        char* temp;
        if(state == -4)
        {
            tem = ui->lineEdit->text();
            QByteArray ba = tem.toLatin1(); // must
            temp = ba.data();
            timelimit = atoi(temp);
            qDebug()<<timelimit;
            startDfs();
            repaint();
            if(state == 0)
            {
                state = PassengerState(state, time0, &len, day);
                repaint();
                timer->start(anHour);
            }
            else if(state == -5)
                state = -4;
        }
    });

    //连接重新开始按钮
    connect(ui->pushButton_r, &QPushButton::pressed, [=](){
        if((fp3 = fopen("journal.txt", "w+")) == NULL)
        {
            printf("Can not open journal.txt");
            exit(0);
        }
        state = -1;
        for(int i = 0; i < citynum; i++)
            flag[i] = 0;
        length = 0;
        TotalLength = 0;
        time1 = 0;
        TotalTime = 0;
        time0 = 0;
        day = 1;
        len = 0;
        TotalRisk = nan;
        connectBut();
        timer->start(anHour);
        ui->stop->setText("暂停");
        stop_flag = 0;
    });

    //连接暂停/继续按钮
    connect(ui->stop, &QPushButton::pressed, [=](){
        if(state >= 0)
        {
            if(stop_flag == 0)
            {
                timer->stop();
                ui->stop->setText("继续");
                stop_flag = 1;
            }
            else if(stop_flag == 1)
            {
                timer->start(anHour);
                ui->stop->setText("暂停");
                stop_flag = 0;
            }
        }
    });

    //连接加速、减速按钮
    connect(ui->quick, &QPushButton::pressed, [=](){
        if(anHour > 500 && anHour <= 10000)
            anHour -= 500;
    });
    connect(ui->slow, &QPushButton::pressed, [=](){
        if(anHour >= 500 && anHour < 10000)
            anHour += 500;
    });

}

void MainWindow::startDfs()//调用dfs()函数，判断是否找到满足要求的路线
{
    starttime = time0;
    success = dfs(startcity, starttime, arrivecity, 0);
    if(success == 0)
    {
        state = -5;
        printf("Can not find a qualified path.\n");
    }
    else
    {
        state = 0;
        printf("Start time: Day1 %d:00\n", starttime);
        printf("Start city: %s\n", city[startcity].name);
        printf("Destination: %s\n", city[arrivecity].name);
        if(timelimit == nan)
            printf("Travel strategy: No time limit, lowest risk\n\n");
        else
            printf("Travel strategy: Time limit: %dh\n\n", timelimit);

        fprintf(fp3, "Start time: Day1 %d:00\n", starttime);
        fprintf(fp3, "Start city: %s\n", city[startcity].name);
        fprintf(fp3, "Destination: %s\n", city[arrivecity].name);
        if(timelimit == nan)
            fprintf(fp3, "Travel strategy: No time limit, lowest risk\n\n");
        else
            fprintf(fp3, "Travel strategy: Time limit: %dh\n\n", timelimit);
        startday = day;
    }
}

void MainWindow::disconnectBut()//断开城市按钮
{
    disconnect(ui->pushButton_1, &QPushButton::clicked, this, &MainWindow::chooseCity1);
    disconnect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::chooseCity2);
    disconnect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::chooseCity3);
    disconnect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::chooseCity4);
    disconnect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::chooseCity5);
    disconnect(ui->pushButton_6, &QPushButton::clicked, this, &MainWindow::chooseCity6);
    disconnect(ui->pushButton_7, &QPushButton::clicked, this, &MainWindow::chooseCity7);
    disconnect(ui->pushButton_8, &QPushButton::clicked, this, &MainWindow::chooseCity8);
    disconnect(ui->pushButton_9, &QPushButton::clicked, this, &MainWindow::chooseCity9);
    disconnect(ui->pushButton_10, &QPushButton::clicked, this, &MainWindow::chooseCity10);
    repaint();
}


void MainWindow::connectBut()//连接城市按钮
{
    connect(ui->pushButton_1, &QPushButton::clicked, this, &MainWindow::chooseCity1);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::chooseCity2);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::chooseCity3);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::chooseCity4);
    connect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::chooseCity5);
    connect(ui->pushButton_6, &QPushButton::clicked, this, &MainWindow::chooseCity6);
    connect(ui->pushButton_7, &QPushButton::clicked, this, &MainWindow::chooseCity7);
    connect(ui->pushButton_8, &QPushButton::clicked, this, &MainWindow::chooseCity8);
    connect(ui->pushButton_9, &QPushButton::clicked, this, &MainWindow::chooseCity9);
    connect(ui->pushButton_10, &QPushButton::clicked, this, &MainWindow::chooseCity10);
    repaint();
}

void MainWindow::chooseCity1()
{
    if(state == -1)
    {
        startcity = 0;
        state = -2;
        disconnect(ui->pushButton_1, &QPushButton::clicked, this, &MainWindow::chooseCity1);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 0;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity2()
{
    if(state == -1)
    {
        startcity = 1;
        state = -2;
        disconnect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::chooseCity2);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 1;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity3()
{
    if(state == -1)
    {
        startcity = 2;
        state = -2;
        disconnect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::chooseCity3);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 2;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity4()
{
    if(state == -1)
    {
        startcity = 3;
        state = -2;
        disconnect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::chooseCity4);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 3;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity5()
{
    if(state == -1)
    {
        startcity = 4;
        state = -2;
        disconnect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::chooseCity5);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 4;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity6()
{
    if(state == -1)
    {
        startcity = 5;
        state = -2;
        disconnect(ui->pushButton_6, &QPushButton::clicked, this, &MainWindow::chooseCity6);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 5;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity7()
{
    if(state == -1)
    {
        startcity = 6;
        state = -2;
        disconnect(ui->pushButton_7, &QPushButton::clicked, this, &MainWindow::chooseCity7);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 6;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity8()
{
    if(state == -1)
    {
        startcity = 7;
        state = -2;
        disconnect(ui->pushButton_8, &QPushButton::clicked, this, &MainWindow::chooseCity8);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 7;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity9()
{
    if(state == -1)
    {
        startcity = 8;
        state = -2;
        disconnect(ui->pushButton_9, &QPushButton::clicked, this, &MainWindow::chooseCity9);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 8;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::chooseCity10()
{
    if(state == -1)
    {
        startcity = 9;
        state = -2;
        disconnect(ui->pushButton_10, &QPushButton::clicked, this, &MainWindow::chooseCity10);
        repaint();
    }
    else if(state == -2)
    {
        arrivecity = 9;
        state = -3;
        disconnectBut();
    }
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix1, pix2, pix3, pix4, pix5;
    QPen pen1;
    char time2[10], day2[10]="第", risk1[10];
    QFont font;
    char cityname[][20]={"北京","哈尔滨","沈阳","广州","上海","西安","重庆","昆明","乌鲁木齐","拉萨"};

    pix1.load(":/soucre/China_map1.jpg");
    pix2.load(":/soucre/plane.jpg");
    pix3.load(":/soucre/bus.jpg");
    pix4.load(":/soucre/train.jpg");
    pix5.load(":/soucre/person.jpg");

    //背景界面
    painter.drawPixmap(width, 30, Width-300, Height-30, pix1);
    QBrush brush(Qt::cyan);
    brush.setColor(QColor(135,206,250));
    painter.setBrush(brush);
    painter.drawRect(Width+width-300, 30, 300, Height-30);

    brush.setColor(QColor(240,248,255));
    painter.setBrush(brush);
    painter.drawRect(Width+width-300,30,300,60);
    painter.drawRect(Width+width-300,30,216,60);
    painter.drawRect(Width+width-300,30,136,60);

    brush.setColor(QColor(0,191,255));
    painter.setBrush(brush);
    painter.drawRect(width+950,110,240,220);
    brush.setColor(QColor(240,248,255));
    painter.setBrush(brush);
    painter.drawRect(width+955,115,230,50);//

    brush.setColor(QColor(0,191,255));
    painter.setBrush(brush);
    painter.drawRect(Width+width-300,345,300,165);
    painter.drawRect(Width+width-300,355,200,100);
    brush.setColor(QColor(240,248,255));
    painter.setBrush(brush);
    painter.drawRect(Width+width-300,355,200,50);
    brush.setColor(QColor(0,191,255));
    painter.setBrush(brush);
    painter.drawRect(Width+width-100,345,100,165);

    brush.setColor(QColor(135,206,250));
    painter.setBrush(brush);
    painter.drawRect(Width+width-300,505,300,180);
    brush.setColor(QColor(240,248,255));
    painter.setBrush(brush);
    painter.drawRect(Width+width-210,505,210,180);

    brush.setColor(QColor(135,206,250));
    painter.setBrush(brush);
    painter.drawRect(Width+width-300,565,300,185);
    brush.setColor(QColor(240,248,255));
    painter.setBrush(brush);
    painter.drawRect(Width+width-295,570,290,175);

    brush.setColor(QColor(240,248,255));
    painter.setBrush(brush);
    painter.drawRect(width+700,Height-40,220,40);
    painter.drawRect(width+0,Height-175,290,175);
    brush.setColor(QColor(135,206,250));
    painter.setBrush(brush);
    painter.drawRect(width+0,Height-175,290,38);

    font.setPointSize(8);
    font.setFamily("Microsoft YaHei");
    font.setLetterSpacing(QFont::AbsoluteSpacing,0);
    painter.setFont(font);
    painter.drawText(QRect(width+715,Height-30,200,30), "Developed by Zicheng Wang");


    //旅行表
    //表格界面
    brush.setColor(QColor(135,206,250));
    painter.setBrush(brush);
    painter.drawRect(0, 30, width, height-30);

    brush.setColor(QColor(240,248,255));
    painter.setBrush(brush);
    painter.drawRect(0, 30, width, 60);
    font.setFamily("Microsoft YaHei");
    font.setPointSize(16);
    painter.setFont(font);
    font.setLetterSpacing(QFont::AbsoluteSpacing,0);
    painter.drawText(QRect(125, 45, 300, 50), "行 程 安 排");

    painter.drawRect(10, 100, 380, 600);
    painter.drawRect(10, 100, 380, 500);
    painter.drawRect(10, 100, 380, 400);
    painter.drawRect(10, 100, 380, 300);
    painter.drawRect(10, 100, 380, 200);
    painter.drawRect(10, 100, 380, 100);

    //表格内容
    int i, day = 1, lasttime = -1;
    for(i = 0; i < TotalLength; i++)
    {
        //出发地、出发时间、到达地、到达时间
        font.setPointSize(12);
        painter.setFont(font);
        painter.drawText(QRect(157,120+i*100,200,100), cityname[plan[i].StartCity]);
        painter.drawText(QRect(240,120+i*100,200,100), "==>");
        painter.drawText(QRect(290,120+i*100,200,100), cityname[plan[i].ArriveCity]);

        font.setPointSize(10);
        painter.setFont(font);
        char a[30]="第", b[30]="第", temp[20];
        if(plan[i].StartTime < lasttime)
            day++;
        itoa(day, temp, 10);
        strcat(a, temp);
        strcat(a, "天 ");
        itoa(plan[i].StartTime, temp, 10);
        strcat(a, temp);
        strcat(a, ":00");
        painter.drawText(QRect(160,160+i*100,200,100), a);

        if(plan[i].ArriveTime < plan[i].StartTime)
            day++;
        itoa(day, temp, 10);
        strcat(b, temp);
        strcat(b, "天 ");
        itoa(plan[i].ArriveTime, temp, 10);
        strcat(b, temp);
        strcat(b, ":00");
        painter.drawText(QRect(280,160+i*100,200,100), b);

        lasttime = plan[i].ArriveTime;

        //交通工具
        if(strcmp(plan[i].type, "plane") == 0)
            painter.drawPixmap(58, 108+i*100, 50, 50, pix2);
        else if(strcmp(plan[i].type, "car") == 0)
            painter.drawPixmap(45, 120+i*100, 70, 34, pix3);
        else if(strcmp(plan[i].type, "train") == 0)
            painter.drawPixmap(50, 120+i*100, 70, 35, pix4);
        painter.drawText(QRect(35,160+i*100,200,100), "班次:");
        painter.drawText(QRect(85,160+i*100,200,100), plan[i].num);
    }

    //顶部时间
    font.setPointSize(15);
    painter.setFont(font);
    itoa(time0, time2, 10);
    strcat(time2, ":00");
    itoa(day, &day2[3], 10);
    strcat(day2, "天");
    painter.drawText(QRect(width+930,40,300,100), "当前时间:");
    painter.drawText(QRect(width+1062,40,100,100), day2);
    painter.drawText(QRect(width+1140,40,100,100), time2);


    //文字提示
    font.setPointSize(15);
    painter.setFont(font);
    if(len == TotalLength && state >= 0)
    {
        painter.setPen(QColor(150,160,35));
        painter.drawText(QRect(width+1005,120,300,100), "到达目的地");
    }
    else if(state == -1)
    {
        painter.setPen(QColor(65,105,225));
        painter.drawText(QRect(width+1000,120,300,100), "选择出发地 ↓");
    }

    else if(state == -2)
    {
        painter.setPen(QColor(150,50,205));
        painter.drawText(QRect(width+1000,120,300,100), "选择目的地 ↓");
    }
    else if(state >= 0)
    {
        painter.setPen(QColor(250,100,135));
        painter.drawText(QRect(width+1010,120,300,100), "开始行程!");
    }


    //选择时间限制
    font.setPointSize(10);
    painter.setFont(font);
    painter.setPen(QColor(150,50,205));
    if(state == -3)
        painter.drawText(QRect(width+955,365,200,30), "选择限时/不限时↓");
    if(state == -4 || state == -5)
    {
        if(state == -4)
            painter.drawText(QRect(width+950,365,200,30), "输入时限 点击确认↓");
        else
            painter.drawText(QRect(width+940,365,200,30), "时限过短 请重新输入↓");
    }


    //文字显示当前状态
    font.setPointSize(10);
    painter.setFont(font);
    painter.setPen(QColor(0,0,0));
    painter.drawText(QRect(width+925,523,200,30), "旅客状态：");
    if(state == 2 || (len == TotalLength && state >= 0))
    {
        char text[30] = "到达";
        if(len == TotalLength && state >= 0)
        {
            strcat(text, cityname[plan[len-1].ArriveCity]);
            strcat(text, " (目的地)");
        }
        else
            strcat(text, cityname[plan[len].ArriveCity]);
        painter.drawText(QRect(width+1020,523,200,30), text);
    }
    else if(state == 0)
    {
        char text[30] = "在";
        strcat(text, cityname[plan[len].StartCity]);
        strcat(text, "等待");
        painter.drawText(QRect(width+1020,523,200,30), text);
    }
    else if(state == 1)
    {
        font.setPointSize(9);
        painter.setFont(font);
        char text[30] = "\0",text1[20] = "班次: \0";
        strcat(text, cityname[plan[len].StartCity]);
        strcat(text, "=>");
        strcat(text, cityname[plan[len].ArriveCity]);
        painter.drawText(QRect(width+1020,513,200,30), text);
        strcat(text1, plan[len].num);
        painter.drawText(QRect(width+1020,538,200,30), text1);
        if(strcmp(plan[len].type, "plane") == 0)
            painter.drawPixmap(width+1150, 510, 50, 50, pix2);
        else if(strcmp(plan[len].type, "car") == 0)
            painter.drawPixmap(width+1140, 520, 70, 34, pix3);
        else if(strcmp(plan[len].type, "train") == 0)
            painter.drawPixmap(width+1145, 520, 60, 30, pix4);

    }


    //底部信息
    font.setPointSize(10);
    painter.setFont(font);
    painter.setPen(QColor(0,0,0));
    painter.drawText(QRect(width+970,575,200,30), "开始时间:");
    painter.drawText(QRect(width+970,600,300,30), "出发城市:");
    painter.drawText(QRect(width+970,625,200,30), "到达时间:");
    painter.drawText(QRect(width+970,650,200,30), "终点城市:");
    painter.drawText(QRect(width+970,675,200,30), "总用时:");
    painter.drawText(QRect(width+970,700,200,30), "总风险值:");
    if(state >= 0)
    {
        itoa(starttime, time2, 10);
        strcat(time2, ":00");
        itoa(startday, &day2[3], 10);
        strcat(day2, "天");
        painter.drawText(QRect(width+1060,575,200,30), day2);
        painter.drawText(QRect(width+1110,575,100,100), time2);
        painter.drawText(QRect(width+1060,600,200,30), cityname[startcity]);
        painter.drawText(QRect(width+1060,650,200,30), cityname[arrivecity]);

        if(len == TotalLength && state >= 0)
        {
            itoa(time0, time2, 10);
            strcat(time2, ":00");
            itoa(day, &day2[3], 10);
            strcat(day2, "天");
            painter.drawText(QRect(width+1060,625,200,30), day2);
            painter.drawText(QRect(width+1110,625,100,100), time2);
            itoa(TotalTime, time2, 10);
            strcat(time2, "小时");
            painter.drawText(QRect(width+1060,675,100,100), time2);
            _gcvt_s(risk1, 10, TotalRisk, 4);
            if(risk1[2] == '.' && risk1[3] == '\0')
            {
                risk1[3] = '0';
                risk1[4] = '\0';
            }
            painter.drawText(QRect(width+1060,700,100,100), risk1);
            if(0 <= TotalRisk && TotalRisk < 30)
                painter.drawText(QRect(width+1110,700,100,100), "(低风险)");
            else if(30 <= TotalRisk && TotalRisk < 60)
                painter.drawText(QRect(width+1110,700,100,100), "(中风险)");
            else
                painter.drawText(QRect(width+1110,700,100,100), "(高风险)");
            font.setPointSize(9);
            painter.setFont(font);
            painter.drawText(QRect(width+960,725,200,30), "(低:<30, 中:30-60, 高:>60)");
        }
    }

    //地图上城市
    int k;
    for(k = 0; k < citynum; k++)
    {
        qDebug()<<city[k].risk<<k<<city[k].x<<city[k].y;
        if(city[k].risk == (float)0.2)
            brush.setColor(QColor(105, 210, 105));
        else if(city[k].risk == (float)0.5)
            brush.setColor(QColor(105, 105, 210));
        else if(city[k].risk == (float)0.9)
            brush.setColor(QColor(210, 105, 105));
        painter.setBrush(brush);
        painter.drawEllipse(QPoint(width+city[k].x, city[k].y), 7, 7);
    }

    //图例
    font.setPointSize(12);
    painter.setFont(font);
    painter.drawText(QRect(width+15,580,200,30), "图例");
    font.setPointSize(10);
    painter.setFont(font);
    brush.setColor(QColor(105, 210, 105));
    painter.setBrush(brush);
    painter.drawEllipse(QPoint(width+170, 645), 7, 7);
    brush.setColor(QColor(105, 105, 210));
    painter.setBrush(brush);
    painter.drawEllipse(QPoint(width+170, 680), 7, 7);
    brush.setColor(QColor(210, 105, 105));
    painter.setBrush(brush);
    painter.drawEllipse(QPoint(width+170, 715), 7, 7);
    painter.drawText(QRect(width+185,632,200,30), "低风险城市");
    painter.drawText(QRect(width+185,667,200,30), "中风险城市");
    painter.drawText(QRect(width+185,702,200,30), "高风险城市");
    painter.drawText(QRect(width+70,632,200,30), "汽车路线");
    painter.drawText(QRect(width+70,667,200,30), "火车路线");
    painter.drawText(QRect(width+70,702,200,30), "飞机路线");
    pen1.setWidth(5);
    pen1.setStyle(Qt::DotLine);
    pen1.setColor(QColor(255,0,0));
    painter.setPen(pen1);
    painter.drawLine(QPoint(width+10, 645), QPoint(width+60, 645));
    pen1.setColor(QColor(0,150,100));
    painter.setPen(pen1);
    painter.drawLine(QPoint(width+10, 680), QPoint(width+60, 680));
    pen1.setColor(QColor(0,0,255));
    painter.setPen(pen1);
    painter.drawLine(QPoint(width+10, 715), QPoint(width+60, 715));


    //地图上路线
    i = 0;
    while(i < len)
    {
        if(strcmp(plan[i].type, "car") == 0)
            pen1.setColor(QColor(255,0,0));
        else if(strcmp(plan[i].type, "train") == 0)
            pen1.setColor(QColor(0,150,100));
        else if(strcmp(plan[i].type, "plane") == 0)
            pen1.setColor(QColor(0,0,255));
        painter.setPen(pen1);
        linex1 = city[plan[i].StartCity].x;
        liney1 = city[plan[i].StartCity].y;
        linex2 = city[plan[i].ArriveCity].x;
        liney2 = city[plan[i].ArriveCity].y;
        painter.drawLine(QPoint(width+linex1, liney1), QPoint(width+linex2, liney2));
        i++;
    }
    linex1 = city[plan[len].StartCity].x;
    liney1 = city[plan[len].StartCity].y;
    linex2 = city[plan[len].ArriveCity].x;
    liney2 = city[plan[len].ArriveCity].y;
    if(strcmp(plan[len].type, "car") == 0)
        pen1.setColor(QColor(255,0,0));
    else if(strcmp(plan[len].type, "train") == 0)
        pen1.setColor(QColor(0,150,100));
    else if(strcmp(plan[len].type, "plane") == 0)
        pen1.setColor(QColor(0,0,255));
    painter.setPen(pen1);

    if(state == 1 || state == 2)
        painter.drawLine(QPoint(width+linex1, liney1), QPoint(width+linex2, liney2));


    //地图上人物状态
    vehicleX = (linex1 + linex2) / 2;
    vehicleY = (liney1 + liney2) / 2;
    if(state >= 0 && len >= TotalLength)
    {
        linex1 = city[arrivecity].x;
        liney1 = city[arrivecity].y;
        painter.drawPixmap(width+linex1-20, liney1-28, 40, 56, pix5);
    }
    else if(state == 0)
        painter.drawPixmap(width+linex1-20, liney1-28, 40, 56, pix5);
    else if(state == 1)
    {
        if(strcmp(plan[len].type, "plane") == 0)
            painter.drawPixmap(width+vehicleX-30, vehicleY-30, 60, 60, pix2);
        else if(strcmp(plan[len].type, "car") == 0)
            painter.drawPixmap(width+vehicleX-35, vehicleY-17, 70, 34, pix3);
        else if(strcmp(plan[len].type, "train") == 0)
            painter.drawPixmap(width+vehicleX-30, vehicleY-15, 60, 30, pix4);
    }
    else if(state == 2)
        painter.drawPixmap(width+linex2-20, liney2-28, 40, 56, pix5);

}

MainWindow::~MainWindow()
{
    delete ui;
}

