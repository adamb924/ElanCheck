#ifndef EAF_H
#define EAF_H

#include <QList>

#include "timeslot.h"
#include "annotation.h"
#include "sound.h"

class QDomDocument;

class Eaf
{
public:
    Eaf();

    enum PathBehavior {
        TryRelativeThenAbsolute,
        TryAbsoluteThenRelative,
        OnlyUseRelative,
        OnlyUseAbsolute
    };

    QDomDocument* document();
    QList<Annotation>* annotations();
    void setFileChanged(bool changed);
    bool fileChanged();

    bool readEaf(QString filename, PathBehavior pathBehavior = TryRelativeThenAbsolute);
    void readAnnotations(QString id);

    void sendDataToDOM(QString id);

    Sound* sound();
    QString filename() const;
    QStringList* tiers();


private:
    QString mEafFilename;
    QString mWavFilename;

    QStringList mTiers;

    qint64 timeFromId(QString id);
    void readTimeSlots();
    void readTierIds();
    bool readWavFilename(PathBehavior pathBehavior);

    Sound mWavFile;

    QDomDocument *mDocument;
    QList<TimeSlot> aTimeSlots;
    QList<Annotation> aAnnotations;

    bool bFileChanged;
    float mTimeIncrement;
};

#endif // EAF_H
