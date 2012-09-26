#include "eaf.h"

#include <QtXml>
#include <QtGui>
#include <QtMultimedia>

Eaf::Eaf()
{
    mTimeIncrement = 0.001;
}

bool Eaf::readEaf(QString filename, PathBehavior pathBehavior)
{
    mEafFilename = filename;

    bFileChanged = false;

    QFile file(mEafFilename);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    mDocument = new QDomDocument;
    if (!mDocument->setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    readWavFilename(pathBehavior);
    if( mWavFilename.isEmpty() )
        return false;

    mWavFile.readWav(mWavFilename);

    readTimeSlots();
    readTierIds();

    if( mTiers.count() > 0 )
        readAnnotations(mTiers.at(0));

    return true;
}

bool Eaf::readWavFilename(PathBehavior pathBehavior)
{
    // find the media file
    QDomNodeList nameList = mDocument->elementsByTagName("MEDIA_DESCRIPTOR");
    if( nameList.count() == 0 )
        return false;

    // this uses the relative path to the wav file
    QFileInfo info(mEafFilename);
    QString relativePath = nameList.at(0).attributes().namedItem("RELATIVE_MEDIA_URL").toAttr().value();
    relativePath.replace(QRegExp("^file:/"),"");
    QDir dir = info.absoluteDir();
    relativePath = QDir::cleanPath(dir.absoluteFilePath(relativePath));

    // this uses the absolute path to the wav file
    QString absolutePath = nameList.at(0).attributes().namedItem("MEDIA_URL").toAttr().value();
    absolutePath.replace(QRegExp("^file:///"),"");

    switch(pathBehavior)
    {
    case TryRelativeThenAbsolute:
        if( QFile::exists(relativePath) )
            mWavFilename = relativePath;
        else if( QFile::exists(absolutePath) )
            mWavFilename = absolutePath;
        else
            return false;
        break;
    case TryAbsoluteThenRelative:
        if( QFile::exists(absolutePath) )
            mWavFilename = absolutePath;
        else if( QFile::exists(relativePath) )
            mWavFilename = relativePath;
        else
            return false;
        break;
    case OnlyUseRelative:
        mWavFilename = relativePath;
        if( !QFile::exists(relativePath) )
            return false;
        break;
    case OnlyUseAbsolute:
        mWavFilename = absolutePath;
        if( !QFile::exists(absolutePath) )
            return false;
        break;
    }

    return true;
}

void Eaf::readTimeSlots()
{
    QDomNodeList timeslotList = mDocument->elementsByTagName("TIME_SLOT");
    for(int i=0; i<timeslotList.count(); i++)
    {
        QString id = timeslotList.at(i).attributes().namedItem("TIME_SLOT_ID").toAttr().value();
        qint64 time = timeslotList.at(i).attributes().namedItem("TIME_VALUE").toAttr().value().toLong();
        //        qDebug() << id << time;
        aTimeSlots.append( TimeSlot(id,time) );
    }
}

void Eaf::readTierIds()
{
    QDomNodeList tierList = mDocument->elementsByTagName("TIER");
    for(int i=0; i<tierList.count(); i++)
    {
        QString id = tierList.at(i).attributes().namedItem("TIER_ID").toAttr().value();
        mTiers.append(id);
    }
}

void Eaf::readAnnotations(QString id)
{
    QDomNodeList tierList = mDocument->elementsByTagName("TIER");
    for(int i=0; i<tierList.count(); i++)
    {
        if( id == tierList.at(i).attributes().namedItem("TIER_ID").toAttr().value() )
        {
            aAnnotations.clear();
            QDomNodeList annotationList = tierList.at(i).toElement().elementsByTagName("ALIGNABLE_ANNOTATION");
            for(int j=0; j<annotationList.count(); j++)
            {
                QString startId = annotationList.at(j).attributes().namedItem("TIME_SLOT_REF1").toAttr().value();
                QString endId = annotationList.at(j).attributes().namedItem("TIME_SLOT_REF2").toAttr().value();
                qint64 startTime = timeFromId(startId);
                qint64 endTime = timeFromId(endId);
                QString value = annotationList.at(j).toElement().elementsByTagName("ANNOTATION_VALUE").at(0).toElement().text();

                aAnnotations.append( Annotation(value,startId,startTime,endId,endTime));
                aAnnotations.last().mAudioData = mWavFile.mAudioData.mid(mWavFile.bytePositionAtTime(startTime*mTimeIncrement), mWavFile.bytePositionAtTime(endTime*mTimeIncrement)-mWavFile.bytePositionAtTime(startTime*mTimeIncrement));
            }
            return;
        }
    }
}

QDomDocument* Eaf::document()
{
    return mDocument;
}

QList<Annotation>* Eaf::annotations()
{
    return &aAnnotations;
}

void Eaf::setFileChanged(bool changed)
{
    bFileChanged = changed;
}

bool Eaf::fileChanged()
{
    return bFileChanged;
}

void Eaf::sendDataToDOM(QString id)
{
    if( aAnnotations.count() < 1 )
        return;

    QDomNodeList tierList = mDocument->elementsByTagName("TIER");
    for(int i=0; i<tierList.count(); i++)
    {
        if( id == tierList.at(i).attributes().namedItem("TIER_ID").toAttr().value() )
        {
            QDomNodeList annotationList = mDocument->elementsByTagName("ALIGNABLE_ANNOTATION");
            for(int i=0; i<annotationList.count(); i++)
            {
                QDomElement ann_val = annotationList.at(i).firstChildElement("ANNOTATION_VALUE");

                QDomElement newNodeTag = mDocument->createElement("ANNOTATION_VALUE");
                QDomText newNodeText = mDocument->createTextNode(aAnnotations.at(i).mValue);
                newNodeTag.appendChild(newNodeText);

                annotationList.at(i).toElement().replaceChild(newNodeTag, ann_val);
            }
            return;
        }
    }
}

qint64 Eaf::timeFromId(QString id)
{
    for(int i=0; i<aTimeSlots.count(); i++)
        if( aTimeSlots.at(i).mId == id )
            return aTimeSlots.at(i).mTime;
    return -1;
}

Sound* Eaf::sound()
{
    return &mWavFile;
}

QString Eaf::filename() const
{
    return mEafFilename;
}

QStringList* Eaf::tiers()
{
    return &mTiers;
}
