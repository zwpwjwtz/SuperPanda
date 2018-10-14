#include <QMetaType>
#include "filecleaner.h"
#include "filecleaner_p.h"


FileCleaner::FileCleaner()
{
    this->d_ptr = new FileCleanerPrivate(this);
}

FileCleaner::~FileCleaner()
{
    delete this->d_ptr;
}

FileCleaner::State FileCleaner::currentState()
{
    Q_D(FileCleaner);
    return d->currentState;
}

QSet<FileCleaner::ScanCategory> FileCleaner::targetCategories()
{
    Q_D(FileCleaner);
    return d->targetCateogries;
}

bool FileCleaner::scan()
{
    QSet<ScanCategory> categories;
    categories.insert(ScanAppCache);
    categories.insert(ScanDownloadPackage);
    categories.insert(ScanUnusedPackageConf);
    return scanByCategory(categories);
}

bool FileCleaner::scanByCategory(const QSet<ScanCategory>& categories)
{
    Q_D(FileCleaner);

    if (categories.isEmpty())
        return false;

    d->currentState = Scanning;
    d->startTime = QDateTime::currentDateTime();
    emit d->startScanningWorker(categories);
    return true;
}

FileCleaner::ScanReportType FileCleaner::getScanReport()
{
    Q_D(FileCleaner);
    return d->scanReport;
}

bool FileCleaner::clean()
{
    Q_D(FileCleaner);
    return cleanByScanReport(d->scanReport);
}

bool FileCleaner::cleanByScanReport(const ScanReportType& report)
{
    Q_D(FileCleaner);

    if (report.entries.count() < 1)
        return false;

    d->currentState = Cleaning;
    emit d->startCleaningWorker(report);
    d->startTime = QDateTime::currentDateTime();
    return true;
}

FileCleaner::CleaningReportType FileCleaner::getCleaningReport()
{
    Q_D(FileCleaner);
    return d->cleanReport;
}

void FileCleaner::stop()
{
    Q_D(FileCleaner);

    d->worker.stop();
    if (d->currentState == Scanning)
        d->scanReport.successful = false;
    else if (d->currentState == Cleaning)
        d->cleanReport.successful = false;
    d->currentState = Stopped;
}

FileCleanerPrivate::FileCleanerPrivate(FileCleaner* parent)
{
    this->q_ptr = parent;
    currentState = FileCleaner::None;

    qRegisterMetaType<QSet<FileCleaner::ScanCategory>>();
    qRegisterMetaType<FileCleaner::ScanReportType>();
    qRegisterMetaType<FileCleaner::ScanCategory>();
    worker.moveToThread(&workerThread);

    // Scanning task
    connect(this,
            SIGNAL(startScanningWorker(QSet<FileCleaner::ScanCategory>)),
            &worker,
            SLOT(startScanning(QSet<FileCleaner::ScanCategory>)));
    connect(&worker,
            SIGNAL(scanningStageStarted(FileCleaner::ScanCategory)),
            this,
            SLOT(workerScanStageStarted(FileCleaner::ScanCategory)));
    connect(&worker,
            SIGNAL(scanningStageFinished(FileCleaner::ScanCategory,int,qint64)),
            this,
            SLOT(workerScanStageFinished(FileCleaner::ScanCategory,int,qint64)));
    connect(&worker,
            SIGNAL(scanningFinished()),
            this,
            SLOT(workerFinished()));

    // Cleaning task
    connect(this,
            SIGNAL(startCleaningWorker(FileCleaner::ScanReportType)),
            &worker,
            SLOT(startCleaning(FileCleaner::ScanReportType)));
    connect(&worker,
            SIGNAL(cleaningStageStarted(QString)),
            this,
            SLOT(workerCleaningStageStarted(QString)));
    connect(&worker,
            SIGNAL(cleaningStageFinished(FileCleaner::ScanCategory,
                                         QString,
                                         qint64)),
            this,
            SLOT(workerCleaningStageFinished(FileCleaner::ScanCategory,
                                             QString,
                                             qint64)));
    connect(&worker,
            SIGNAL(cleaningFinished()),
            this,
            SLOT(workerFinished()));

    workerThread.start();
}

FileCleanerPrivate::~FileCleanerPrivate()
{
    workerThread.quit();
    workerThread.wait();
}

void FileCleanerPrivate::workerScanStageStarted(ScanCategory category)
{
    Q_Q(FileCleaner);

    if (currentState == State::Scanning)
        emit q->scanStaged(category);
}

void FileCleanerPrivate::workerScanStageFinished(ScanCategory stage,
                                                 int itemCount,
                                                 qint64 cleanableSize)
{
    Q_Q(FileCleaner);

    if (currentState == State::Scanning)
    {
        FileCleaner::ReportEntry entry;
        entry.finished = true;
        entry.category = stage;
        entry.totalSize = cleanableSize;
        scanReport.entries.push_back(entry);
        emit q->scanStageFinished(entry);
    }
}

void FileCleanerPrivate::workerCleaningStageStarted(QString ID)
{
    Q_Q(FileCleaner);

    if (currentState == State::Scanning)
        emit q->cleaningStaged(ID);
}

void FileCleanerPrivate::workerCleaningStageFinished(ScanCategory stage,
                                                     QString ID,
                                                     qint64 cleanableSize)
{
    Q_Q(FileCleaner);

    if (currentState == State::Cleaning)
    {
        FileCleaner::ReportEntry entry;
        FileCleaner::ReportEntryDetails details;
        details.ID = ID;
        details.cleanableSize = cleanableSize;

        int i;
        for (i=0; i<cleanReport.entries.count(); i++)
        {
            if (cleanReport.entries[i].category == stage)
            {
                // Category found; append details to it
                cleanReport.entries[i].totalSize += cleanableSize;
                cleanReport.entries[i].details.push_back(details);
                break;
            }
        }
        if (i >= cleanReport.entries.count())
        {
            // No category found; create a new category
            entry.finished = true;
            entry.category = stage;
            entry.totalSize = cleanableSize;
            entry.details.push_back(details);
            cleanReport.entries.push_back(entry);
        }
        emit q->cleaningStageFinished(entry);
    }
}

void FileCleanerPrivate::workerFinished()
{
    Q_Q(FileCleaner);

    scanReport.successful = true;
    scanReport.timeElapsed = startTime.msecsTo(QDateTime::currentDateTime());

    if (currentState == State::Scanning)
        emit q->scanFinished();
    else if (currentState == State::Cleaning)
        emit q->cleaningFinished();
    currentState = State::Finished;
}
