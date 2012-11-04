#include "annotation.h"

Annotation::Annotation(QString value, QString startId, qint64 startTime, QString endId, qint64 endTime)
{
    mValue = value;
    mStartId = startId;
    mEndId = endId;
    mEndTime = endTime;
    mFlag = false;
}

void Annotation::setFlag(bool value)
{
    mFlag = value;
}

bool Annotation::isFlagged() const
{
    return mFlag;
}
