#ifndef STREAM_H
#define STREAM_H

#include <math.h>
#include <QAudioOutput>
#include <QByteArray>
#include <QIODevice>
#include <QMainWindow>
#include <QObject>
#include <QTimer>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QAudioInput>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QtCore>
#include <QMutexLocker>
#include <QCryptographicHash>
#include <qmath.h>
#include <qendian.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <QMainWindow>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <CkCrypt2.h>
#include <CkByteData.h>
#define BUFREC 4096
class VWorker;

const int BufferSize = 32768;
const int DurationSeconds  = 1;
const int ToneSampleRateHz = 600;
const int DataSampleRateHz = 44100;

QT_BEGIN_NAMESPACE
namespace Ui {
    static bool start = 1;
    static int fd, new_socket, valread;
    static struct sockaddr_in address;
    static int addrlen = sizeof(address);
    static bool server = 0;
    static std::vector<char> vector;
    static QMutex mutex;
    static CkCrypt2 *cry;
    class VWorker;
}
QT_END_NAMESPACE



class AudioInfo : public QIODevice
{
    Q_OBJECT
public:
    AudioInfo(const QAudioFormat &format, QObject *parent);
    ~AudioInfo(){}

    void start();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    const QAudioFormat m_format;

};

class AudioStream : public QObject
{
    Q_OBJECT

public:
    AudioStream(void *main, CkCrypt2 *crypt);
    ~AudioStream(){}

    void startRec();
    void stopRec();

    /*void setPw(std::string pw){
        QByteArray arr(QByteArray::fromRawData(pw.c_str(), 32));

        Ui::pw = QCryptographicHash::hash(arr, QCryptographicHash::Sha256).constData();
        std::cout <<  Ui::pw<<"\n";
    }
*/
private slots:
    void readMore();

private:
    //Ui::MainWindow *ui;
    std::ofstream ofile;
    // Owned by layout
    AudioInfo *m_audioInfo;
    int m_fd;

    //QAudioDeviceInfo m_device;
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QIODevice *m_input;
    QIODevice *m_iodevice;
    QByteArray m_buffer;
    qint64 bytes=0;

public slots:
    void onReady(int fd){
        m_fd = fd;
        qInfo()<<m_fd;
    }

};

class Generator : public QIODevice
{
    Q_OBJECT

public:
    Generator(const QAudioFormat &format, qint64 durationUs, int sampleRate, QObject *parent);
    ~Generator();

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

private:
    qint64 m_pos;
    QByteArray m_buffer;
};


class FromStream : public QObject
{
    Q_OBJECT

public:
    FromStream(void *main, CkCrypt2 *crypt, const char *ip, int port);
    ~FromStream(){}

    void startS(int port);
    int startC(const char *ip,int port);

    void startPlay();
    void stopPlay();

    VWorker* getWorker() {
        return worker;
    }

/*    void setPw(std::string pw){
        //std::cout<<pw<<"\n";
        QByteArray arr(QByteArray::fromRawData(pw.c_str(), 32));

        Ui::pw = QCryptographicHash::hash(arr, QCryptographicHash::Sha256).constData();
        std::cout << Ui::pw<<"\n";
    }*/

    std::vector<char> slicer(const std::vector<char> &v,  int X, int Y) {
        auto first = v.begin() + X;
        auto last = v.begin() + Y + 1;

        std::vector<char> vector(first, last);

        return vector;
    }

private:
    //Ui::MainWindow *ui;
    QThread *ead;
    VWorker *worker;
    QTimer *m_pushTimer;

    // Owned by layout
    QAudioDeviceInfo m_device;
    Generator *m_generator;
    QAudioOutput *m_audioOutput;
    QIODevice *m_output; // not owned
    QAudioFormat m_format;

    QByteArray m_buffer;
    //std::ifstream m_file;
    std::ofstream m_ofile;
    QMutex *mutex;
    quint64 cut;

private slots:
    void pushTimerExpired();
    //void on_pushButton_3_clicked();

};


class VWorker : public QObject {
    Q_OBJECT
public:

    VWorker(){
        m_ofile.open("/home/yee/Desktop/in.wav", std::ios::binary);
    }

    ~VWorker(){}

    std::ofstream m_ofile;

public slots:
    void process();

signals:
    void fdReady(int fd);

};


#endif // STREAM_H
