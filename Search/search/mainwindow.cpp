#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <string>
#include <iostream>

//#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/video/video.hpp>

#define PATH "/home/ly0/storage/"
#define TYPE ".png"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("The Viros");

    connect(ui->cancel_btn,SIGNAL(clicked(bool)),this,SLOT(close()));
    //connect(ui->ok_btn,SIGNAL(clicked(bool)),this,SLOT(OK_btn_onclick()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_ok_btn_clicked() //lấy dữ liệu
{

    QString plate = ui->license_plate->text();
    qDebug()<<plate;
   if(plate.isEmpty() | (plate.size()!=8 && plate.size() != 9))
    {
       QMessageBox::information(this,"Nhập sai","Vui lòng nhập lại.");
       return;
   }
    QTextStream out(stdout);
    QFile file("logfile.txt");
    if(!file.exists()){
          qDebug()<<"File not exists.";
    }
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"Khong doc duoc file.";
    }
    QTextStream in(&file);
    int flag=0;
    while(!in.atEnd())
    {
        QString line = in.readLine();
        if (line.startsWith(plate)){ qDebug()<<"Có trong log file.";
            flag=flag+1;
            int n = line.size();
            QString location=line;

            int i = 9-plate.size();
            location[9-i] = '_';
            //int j=9-i;
            QString time;
            for(int j = 10-i;j < n; j++){
                    time+=location[j];
            }
            std::string t=time.toUtf8().constData();
            location = PATH+location+TYPE;
            std::string loc = location.toUtf8().constData();

            cv::Mat img = cv::imread(loc);
            cv::namedWindow(t,cv::WINDOW_AUTOSIZE);
            cv::imshow(t, img);

        }
    }
    if(flag==0) QMessageBox::information(this,"KHÔNG TÌM THẤY","Biển số bạn muốn tìm không có trong log file.");
 }


