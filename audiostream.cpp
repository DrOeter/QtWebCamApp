#include "stream.h"
#include "ui_mainwindow.h"

void AudioStream::startRec(){
        m_audioInput->stop();

        m_input = m_audioInput->start();
        connect(m_input, SIGNAL(readyRead()), SLOT(readMore()));
}

void AudioStream::stopRec(){
    ofile.close();
    exit(0);
}

AudioStream::AudioStream(void *main, CkCrypt2 *crypt)
  :   m_audioInfo(0)
  ,   m_audioInput(0)
  ,   m_input(0)
  ,   m_buffer(BUFREC, 0)
{
    ofile.open("/home/yee/Desktop/out.wav",std::ios::binary | std::ios::trunc);

    Ui::cry = crypt;
    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleSize(16);
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }

    m_audioInfo  = new AudioInfo(m_format, (QObject*)main);
    m_audioInput = new QAudioInput(m_format, (QObject*)main);

    m_audioInfo->start();
    m_audioInput->start(m_audioInfo);
}

void AudioStream::readMore()
{
    if (!m_audioInput)  return;
    qint64 len = m_audioInput->bytesReady();

    if (len > BUFREC)  len = BUFREC;
    qint64 l = m_input->read(m_buffer.data(), len);
    CkByteData data;

    bool ok = Ui::cry->EncryptBytes2(&m_buffer.data()[0], l, data);

    ofile.write(m_buffer.constData(), l);
    //qInfo()<<m_fd;

   // unsigned char *out = aes.EncryptECB(&plain[0], plain.size(), key);

    //std::vector<unsigned char> enc (out, out + aes.getLen());

    if(l > 0){
        int left = 0;
        int in = data.getSize();

        std::string dsize = "$";
        if(data.getSize() < 10)  dsize += "000";
        if(data.getSize() >= 10 && data.getSize() <= 99)  dsize += "00";
        if(data.getSize() >= 100 && data.getSize() <= 999)  dsize += "0";
        dsize += std::to_string(data.getSize()) + "$";
        //std::cout << dsize<<std::endl;

        send(m_fd, dsize.c_str(), 6, 0);

        while( in >= 1024 ){
            send(m_fd, &data.getData()[left], 1024, 0);
            //ofile.write(&m_buffer.constData()[left], 1024);
            left += 1024;
            in -= 1024;
            bytes += 1024;
        }
        if(in > 0) {
            send(m_fd, &data.getData()[left], in, 0);
            //ofile.write(&m_buffer.constData()[left], in);
            bytes += in;
        }
    }
    //qInfo()<<bytes;

    //if (l > 0)  m_audioInfo->write(m_buffer.constData(), l);
}

AudioInfo::AudioInfo(const QAudioFormat &format, QObject *parent)
    :   QIODevice(parent)
    ,   m_format(format)

{}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)
    return 0;
}

qint64 AudioInfo::writeData(const char *data, qint64 len)
{
    return len;
}

void AudioInfo::start()
{
    open(QIODevice::WriteOnly);
}
