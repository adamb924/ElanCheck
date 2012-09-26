#include "annotation.h"

Annotation::Annotation(QString value, QString startId, qint64 startTime, QString endId, qint64 endTime)
{
    mValue = value;
    mStartId = startId;
    mEndId = endId;
    mEndTime = endTime;
}
