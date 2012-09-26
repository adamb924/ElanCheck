#ifndef SOUND_H
#define SOUND_H

#include <QtGlobal>
#include <QByteArray>
#include <QtMultimedia>

class Sound
{
public:
    Sound();

    bool readWav(QString filename);

    QAudioFormat getAudioFormat();

    inline qint64 sampleAtTime( float time )
    {
        return (qint64)(time*mSampleRate);
    }

    inline qint64 bytePositionAtTime( float time )
    {
        return (mBitsPerSample/8) * sampleAtTime( time );
    }

    QByteArray mAudioData;


    qint32 mSampleRate;
    qint16 mBitsPerSample;
    qint16 mNChannels;
};

#endif // SOUND_H
