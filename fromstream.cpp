#include "stream.h"
#include "ui_mainwindow.h"

void FromStream::startPlay(){
    qInfo()<<"TESTTEST";
    m_pushTimer->stop();
    m_audioOutput->stop();
    m_output = m_audioOutput->start();
    m_pushTimer->start( 20 );
}

void FromStream::stopPlay(){
    Ui::start = 0;

    qInfo()<< Ui::vector.size();

    m_ofile.open("/home/yee/Desktop/in.wav", std::ios::binary);

    m_ofile.write((const char*)&Ui::vector[0], Ui::vector.size());

    m_ofile.close();
    exit(0);
}

void FromStream::startS(int port) {
    int opt = 1;

    // Creating socket file descriptor
    if ((Ui::fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return;
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(Ui::fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))  {
        perror("setsockopt");
        return;
    }
    Ui::address.sin_family = AF_INET;
    Ui::address.sin_addr.s_addr = INADDR_ANY;
    Ui::address.sin_port = htons( port );

    // Forcefully attaching socket to the port 8080
    if (bind(Ui::fd, (struct sockaddr *)&Ui::address, sizeof(Ui::address))<0) {
        perror("bind failed");
        return;
    }
    if (listen(Ui::fd, 3) < 0) {
        perror("listen");
        return;
    }
    Ui::server = 1;
}

int FromStream::startC(const char* ip, int port) {
    struct sockaddr_in address;

    if ((Ui::fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\n Socket creation error \n");
        return 3;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons( port );

    if(inet_pton(AF_INET, ip, &address.sin_addr) <= 0)  {
        perror("\nInvalid address/ Address not supported \n");
        return 2;
    }

    if(::connect(Ui::fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("\nConnection Failed \n");
        ::close(Ui::fd);
        return 1;
    }
    return 0;
}

FromStream::FromStream(void *main, CkCrypt2 *crypt, const char *ip, int port)
    :   m_pushTimer(new QTimer((QObject*)main))
    ,   m_device(QAudioDeviceInfo::defaultOutputDevice())
    ,   m_generator(0)
    ,   m_audioOutput(0)
    ,   m_output(0)
    ,   m_buffer(BufferSize, 0)
{
    if(startC(ip, port) == 1) startS( port );

    Ui::cry = crypt;

    connect(m_pushTimer, SIGNAL(timeout()), SLOT(pushTimerExpired()));

    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }

    m_generator = new Generator(m_format, DurationSeconds*1000000, ToneSampleRateHz, (QObject*)main);

    delete m_audioOutput;
    m_audioOutput = 0;
    m_audioOutput = new QAudioOutput(m_format, (QObject*)main);
    m_generator->start();
    m_audioOutput->start(m_generator);

    ead= new QThread();
    worker= new VWorker();
    worker->moveToThread(ead);
    connect(ead, SIGNAL (started()), worker, SLOT (process()));
    ead->start();

    qInfo()<< "connected";
}

Generator::Generator(const QAudioFormat &format, qint64 durationUs, int sampleRate, QObject *parent)
    :   QIODevice(parent),   m_pos(0)
{
}

Generator::~Generator()
{
}

void Generator::start()
{
    open(QIODevice::ReadOnly);
}

void Generator::stop()
{
    m_pos = 0;
    close();
}

qint64 Generator::readData(char *data, qint64 len)
{
    qint64 total = 0;
    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
            memcpy(data + total, m_buffer.constData() + m_pos, chunk);
            m_pos = (m_pos + chunk) % m_buffer.size();
            total += chunk;
        }
    }
    return total;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}

qint64 Generator::bytesAvailable() const
{
    return m_buffer.size() + QIODevice::bytesAvailable();
}

void FromStream::pushTimerExpired()
{
    if (m_audioOutput && m_audioOutput->state() != QAudio::StoppedState) {
        int chunks = m_audioOutput->bytesFree()/m_audioOutput->periodSize();
        const quint32 psize = m_audioOutput->periodSize();

        while (chunks) {
           m_output->write(&Ui::vector[0], psize);

           Ui::mutex.lock();
           if(cut != Ui::vector.size())  qInfo()<<cut<<" "<<Ui::vector.size();

           const std::vector<char> &rest = slicer(Ui::vector, psize, (Ui::vector.size() - 1));

           Ui::vector.clear();
           Ui::vector = rest;
           cut = Ui::vector.size();
           Ui::mutex.unlock();

           --chunks;
        }
    }
}

void VWorker::process()  {
    if (Ui::server == 1)  {
        Ui::new_socket = accept(Ui::fd, (struct sockaddr *)&Ui::address, (socklen_t*)&Ui::addrlen );
        Ui::fd = Ui::new_socket;
    }
    else if(Ui::server == 0) Ui::new_socket = Ui::fd;

    qInfo() << "yee"<<Ui::new_socket<<Ui::fd;

    usleep(100000);
    emit fdReady(Ui::fd);

    char buf[1024];
    int in;

    while(1) {
        if(!Ui::start) break;
        std::vector<char> acum;

        recv(Ui::new_socket, buf, 6, 0);
        const int dsize = in = atoi(std::string(buf).substr(1,4).c_str());

        while( in >= 1024 ) {
            recv(Ui::new_socket, buf, 1024, 0);
            m_ofile.write(buf, 1024);

            for(auto i: buf) {
                acum.push_back(i);
                if(i != acum.back()) qInfo()<<"yeet";
            }

            in -= 1024;
        }
        if(in > 0) {
            recv(Ui::new_socket, buf, in, 0);
            m_ofile.write(buf, in);

            for(int i=0;i < in ;i++){
                acum.push_back(buf[i]);
                if(buf[i] != acum.back()) qInfo()<<"yeet";
            }
        }
        CkByteData data;
        bool ok2 = Ui::cry->DecryptBytes2(&acum[0], dsize, data);

        Ui::mutex.lock();
        for(int i=0; i < dsize ;i++)
            Ui::vector.push_back(data.getData()[i]);

        Ui::mutex.unlock();
    }
}
