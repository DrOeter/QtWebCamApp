#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QString>
#include <QThread>
#include <QTextCodec>
#include <QElapsedTimer>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "stream.h"
using namespace cv;

//using namespace std;
class MainWindow;
class Worker;
namespace Ui {
    class MainWindow;
    class Worker;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    friend class Woker;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void startS(int port);
    int startC(const char* ip, int port);

public slots:
    void onprogress(const QImage &in);

private slots:
    void on_pushButton_open_webcam_clicked();

    void on_pushButton_close_webcam_clicked();

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

    void update_window();

private:
    Ui::MainWindow *ui;
    QThread *ead;
    Worker *worker;
    QTimer *timer;
    VideoCapture cap;

    Mat frame;
    QImage qt_image;
    bool on1 = 0, on2 = 0;
    AudioStream *stream;
    FromStream *fstream;
    CkCrypt2 *cry;
};

class Worker : public QObject {
    Q_OBJECT
public:

    Worker(){ }

    ~Worker(){}

public slots:
    void process();

signals:
    void progress(const QImage &info);
    void finished();
    void started();
    void error(QString err);

private:

};


#endif // MAINWINDOW_H
