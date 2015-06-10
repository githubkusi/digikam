#include "KDCRAW/RActionJob"

#include "dbjob.h"
#include "databaseaccess.h"
#include "databaseparameters.h"
#include "albumdb.h"
#include "imagelister.h"
#include "digikam_export.h"
#include "digikam_debug.h"

namespace Digikam {

DBJob::DBJob()
    : RActionJob()
{
}

DBJob::~DBJob()
{
    this->cancel();
}

// ----------------------------------------------

AlbumsJob::AlbumsJob(AlbumsDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

AlbumsJob::~AlbumsJob()
{
}

void AlbumsJob::run()
{
    if (m_jobInfo->folders)
    {
        QMap<int, int> albumNumberMap = DatabaseAccess().db()->getNumberOfImagesInAlbums();

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << albumNumberMap;
        emit data(ba);
    }
    else
    {
        ImageLister lister;
        lister.setRecursive(m_jobInfo->recursive);
        lister.setListOnlyAvailable(m_jobInfo->listAvailableImagesOnly);
        // send data every 200 images to be more responsive
        Digikam::ImageListerJobGrowingPartsSendingReceiver receiver(this, 200, 2000, 100);
        lister.listAlbum(&receiver, m_jobInfo->albumRootId, m_jobInfo->album);
        receiver.sendData();
    }
}

// ----------------------------------------------

DatesJob::DatesJob(DatesDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

DatesJob::~DatesJob()
{
}

void DatesJob::run()
{
    if (m_jobInfo->folders)
    {
        QMap<QDateTime, int> dateNumberMap = DatabaseAccess().db()->getAllCreationDatesAndNumberOfImages();
        QByteArray           ba;
        QDataStream          os(&ba, QIODevice::WriteOnly);
        os << dateNumberMap;
        emit data(ba);
    }
    else
    {
        ImageLister lister;
        lister.setListOnlyAvailable(true);
        // send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);
        lister.listDateRange(&receiver, m_jobInfo->startDate, m_jobInfo->endDate);
        // send rest
        receiver.sendData();
    }
}

// ----------------------------------------------

GPSJob::GPSJob(GPSDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

GPSJob::~GPSJob()
{
}

void GPSJob::run()
{
    if (m_jobInfo->wantDirectQuery)
    {
        QList<QVariant> imagesInfoFromArea =
                DatabaseAccess().db()->getImageIdsFromArea(m_jobInfo->lat1,
                                                           m_jobInfo->lat2,
                                                           m_jobInfo->lng1,
                                                           m_jobInfo->lng2,
                                                           0,
                                                           QLatin1String("rating"));

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << imagesInfoFromArea;
        emit data(ba);
    }
    else
    {
        ImageLister lister;
        lister.setAllowExtraValues(true);
        lister.setListOnlyAvailable(m_jobInfo->listAvailableImagesOnly);
        // send data every 200 images to be more responsive
        ImageListerJobPartsSendingReceiver receiver(this, 200);
        lister.listAreaRange(&receiver,
                             m_jobInfo->lat1,
                             m_jobInfo->lat2,
                             m_jobInfo->lng1,
                             m_jobInfo->lng2);
        // send rest
        receiver.sendData();
    }
}

// ----------------------------------------------

TagsJob::TagsJob(TagsDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

TagsJob::~TagsJob()
{
}

void TagsJob::run()
{
    if (m_jobInfo->folders)
    {
        QMap<int, int> tagNumberMap = DatabaseAccess().db()->getNumberOfImagesInTags();

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << tagNumberMap;
        emit data(ba);
    }
    else if (m_jobInfo->faceFolders)
    {
        QMap<QString, QMap<int, int> > facesNumberMap;
        facesNumberMap[ImageTagPropertyName::autodetectedFace()] =
            DatabaseAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::autodetectedFace());
        facesNumberMap[ImageTagPropertyName::tagRegion()]        =
            DatabaseAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::tagRegion());

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << facesNumberMap;
        emit data(ba);
    }
    else
    {
        Digikam::ImageLister lister;
        lister.setRecursive(m_jobInfo->recursive);
        lister.setListOnlyAvailable(m_jobInfo->listAvailableImagesOnly);
        // send data every 200 images to be more responsive
        Digikam::ImageListerJobPartsSendingReceiver receiver(this, 200);

        if (!m_jobInfo->specialTag.isNull())
        {
            QString searchXml =
                lister.tagSearchXml(m_jobInfo->tagsIds.first(),
                                    m_jobInfo->specialTag,
                                    m_jobInfo->recursive);
            lister.setAllowExtraValues(true); // pass property value as extra value, different binary protocol
            lister.listImageTagPropertySearch(&receiver, searchXml);
        }
        else
        {
            lister.listTag(&receiver, m_jobInfo->tagsIds);
        }

        // finish sending
        receiver.sendData();
    }
}

// ----------------------------------------------

SearchesJob::SearchesJob(SearchesDBJobInfo *jobInfo)
    : DBJob()
{
    m_jobInfo = jobInfo;
}

SearchesJob::~SearchesJob()
{
}

void SearchesJob::run()
{
// bool duplicates = !metaData(QLatin1String("duplicates")).isEmpty();

    // TODO: CHECK WHEN DOES THIS VALUE CHANGE FROM ZERO
    int listingType = 0;

//    if (!ds.atEnd())
//    {
//        ds >> listingType;
//    }

//    Digikam::DatabaseUrl dbUrl(url);
//    QDBusConnection::sessionBus().registerService(QString::fromUtf8("org.kde.digikam.KIO-digikamtags-%1")
//                                                  .arg(QString::number(QCoreApplication::instance()->applicationPid())));
//    Digikam::DatabaseAccess::setParameters((Digikam::DatabaseParameters)dbUrl);

    if (!m_jobInfo->duplicates)
    {
        SearchInfo info = DatabaseAccess().db()->getSearchInfo(m_jobInfo->searchId);

        ImageLister lister;
        lister.setListOnlyAvailable(m_jobInfo->listAvailableImagesOnly);

        if (listingType == 0)
        {
            // send data every 200 images to be more responsive
            ImageListerJobPartsSendingReceiver receiver(this, 200);

            if (info.type == DatabaseSearch::HaarSearch)
            {
                lister.listHaarSearch(&receiver, info.query);
            }
            else
            {
                lister.listSearch(&receiver, info.query);
            }

            if (!receiver.hasError)
            {
                receiver.sendData();
            }
        }
        else
        {
            ImageListerJobReceiver receiver(this);
            // fast mode: limit results to 500
            lister.listSearch(&receiver, info.query, 500);

            if (!receiver.hasError)
            {
                receiver.sendData();
            }
        }
    }
    else
    {
        if (m_jobInfo->albumIds.isEmpty() && m_jobInfo->tagIds.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "No album ids passed for duplicates search";
//            error(KIO::ERR_INTERNAL, i18n("No album ids passed"));
            return;
        }

        // get info about threshold
        // If threshold value cannot be converted from string, we will use 0.4 instead.
        // 40% sound like the minimum value to use to have suitable results.
//        bool   ok;
//        double threshold = thresholdString.toDouble(&ok);

//        if (!ok)
//        {
//            threshold = 0.4;
//        }

//        DuplicatesProgressObserver observer(this);

//        // rebuild the duplicate albums
//        Digikam::HaarIface iface;
//        iface.rebuildDuplicatesAlbums(albumIds, tagIds, threshold, &observer);
    }
}

} // namespace Digikam