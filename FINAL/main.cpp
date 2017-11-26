#include "mainwindow.h"
#include <QApplication>

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/video/video.hpp>
#include <opencv2/ml/ml.hpp>
#include <cstring>

using namespace cv;
using namespace std;



void DetectPlate(Mat frame, Mat &result);
void DetectChar(cv::Mat plateCropped);

/* ==================================================================================================  */
/*                                     MAIN                                                            */
/* ==================================================================================================  */
int main(int argc, char *argv[])
{
    VideoCapture camera(argv[1]);   // receive the fed stream source
    if(!camera.isOpened())          // check if we succeeded
        return -1;

    namedWindow("Video Streamed", WINDOW_NORMAL);
    namedWindow("displayedFrame", WINDOW_NORMAL);
    while(1)
    {
        Mat cameraFrame;
        camera >> cameraFrame;          // get a new frame from camera
        imshow("Video Streamed", cameraFrame);

        Mat displayedFrame;             // the frames that will show processed results

        DetectPlate(cameraFrame, displayedFrame);
        DetectChar(displayedFrame);

        cv::imshow("displayedFrame", displayedFrame);

        if(waitKey(30) == 'c') break;
        waitKey(20);
    }
    waitKey(0);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
/* ==================================================================================================  */
/*                                     DETECT PLATE                                                    */
/* ==================================================================================================  */
void DetectPlate(Mat frame, Mat &result){
    Mat frame_gray;
    CascadeClassifier plate_cascade;
    vector<Rect> rects;

    cvtColor(frame, frame_gray, CV_BGR2GRAY);

    equalizeHist(frame_gray, frame_gray);

    plate_cascade.load("bienso.xml");
    plate_cascade.detectMultiScale(frame_gray, rects, 1.1, 2, 0, cv::Size(0, 0));

    if(rects.size() == 0)
        frame.copyTo(result);
    else{
        Mat mat = frame( rects[0]);
        resize(mat, result, Size((float)mat.cols * 360 / (float)mat.rows, 360));
    }
}

/* ==================================================================================================  */
/*                                     DETECT CHARACTER                                                */
/* ==================================================================================================  */

void DetectChar(cv::Mat plateCropped){
    //dectect ký tự
    CascadeClassifier char_cascade;
    Mat frame_gray, charImage;
    vector<Mat> saveImage;
    cvtColor( plateCropped, frame_gray, CV_BGR2GRAY );
    vector<Rect> rects;

    char_cascade.load("kytu.xml");
    char_cascade.detectMultiScale( frame_gray, rects, 1.1, 2, 0, Size(0, 0) );

    for(int i = 0; i < rects.size(); i++){
        rectangle(plateCropped, rects[i], Scalar(255), 1, 8, 0);
        plateCropped(rects[i]).copyTo(charImage);

        //resize(charImage, charImage, Size(20,48));

        //string name[8];
        //name[i]=(char)(i + 1 + '0');

        //saveImage.push_back(charImage);

        //namedWindow(name[i]);
        //imshow(name[i], charImage);
    }
}
