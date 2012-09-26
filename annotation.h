#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QtGlobal>
#include <QString>

class Annotation
{
public:
    Annotation(QString value, QString startId, qint64 startTime, QString endId, qint64 endTime);

    QString mValue;
    QString mStartId;
    qint64 mStartTime;
    QString mEndId;
    qint64 mEndTime;
    QByteArray mAudioData;
};

#endif // ANNOTATION_H
