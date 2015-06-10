#ifndef DBJOBSMANAGER_H
#define DBJOBSMANAGER_H

#include <QObject>
#include "dbjobinfo.h"
#include "dbjobsthread.h"

namespace Digikam {

class DBJobsManager : public QObject
{
    Q_OBJECT

public:

    explicit DBJobsManager();
    ~DBJobsManager();

    static DBJobsManager* instance();

    DBJobsThread* startAlbumsJobThread(AlbumsDBJobInfo *jInfo);
    DBJobsThread* startDatesJobThread(DatesDBJobInfo *jInfo);
    DBJobsThread* startGPSJobThread(GPSDBJobInfo *jInfo);
    DBJobsThread* startTagsJobThread(TagsDBJobInfo *jInfo);
    DBJobsThread* startSearchesJobThread(SearchesDBJobInfo *jInfo);

private:
    friend class DBJobsManagerCreator;
};

} // namespace Digikam

#endif // DBJOBSMANAGER_H