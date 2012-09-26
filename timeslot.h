#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <QtGlobal>
#include <QString>

class TimeSlot
{
public:
    TimeSlot(QString id, qint64 time);

    QString mId;
    qint64 mTime;
};

#endif // TIMESLOT_H
