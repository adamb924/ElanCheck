#include "sound.h"

#include <QtWidgets>
#include <QtMultimedia>

Sound::Sound()
{
    // defaults:
    mSampleRate = 22050;
    mBitsPerSample = 2;
    mNChannels = 1;
}

bool Sound::readWav(QString filename)
{
    QFile audio_file(filename);
    if(audio_file.open(QIODevice::ReadOnly)) {
        QByteArray header = audio_file.read(44);
        if(QString(header.left(4)) != "RIFF")
        {
            QMessageBox::critical(0,QObject::tr("Error"),QObject::tr("Only WAV files are supported at the moment."));
            return false;
        }

        QDataStream stream(&header, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.skipRawData(22);
        stream >> mNChannels;
        stream >> mSampleRate;
        stream.skipRawData(6);
        stream >> mBitsPerSample;

        audio_file.seek(44); // skip wav header
        mAudioData = audio_file.readAll();
        audio_file.close();

        return true;
    }
    else
    {
        QMessageBox::warning(0,QObject::tr("Warning"),QObject::tr("Could not open %1 because: %2").arg(filename).arg(audio_file.errorString()));
        return false;
    }
}

QAudioFormat Sound::getAudioFormat()
{
    QAudioFormat format;
    format.setSampleSize(mBitsPerSample);
    format.setSampleRate(mSampleRate);
    format.setChannelCount(mNChannels);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);
    return format;
}
