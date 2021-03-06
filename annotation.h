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

    bool isFlagged() const;
    void setFlag(bool value);

private:
    bool mFlag;
};

#endif // ANNOTATION_H
