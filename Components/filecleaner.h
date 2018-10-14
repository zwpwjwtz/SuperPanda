#ifndef FILECLEANER_H
#define FILECLEANER_H

#include <QThread>
#include <QSet>


class FileCleanerPrivate;

class FileCleaner : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FileCleaner)
protected:
    FileCleanerPrivate* d_ptr;

public:
    enum State
    {
        None = 0,
        Scanning = 1,
        Cleaning = 2,
        Stopped = 3,
        Finished = 4
    };
    enum ScanCategory
    {
        ScanAppCache = 1,
        ScanDownloadPackage = 2,
        ScanUnusedPackageConf = 3
    };

    struct ReportEntryDetails
    {
        QString ID;
        QString path;
        qint64 cleanableSize;
    };
    struct ReportEntry
    {
        ScanCategory category;
        bool finished;
        qint64 totalSize;
        QList<ReportEntryDetails> details;
    };
    struct ScanReportType
    {
        bool successful;
        int timeElapsed;
        QList<ReportEntry> entries;
    };
    struct CleaningReportType
    {
        int timeElapsed;
        bool successful;
        QList<ReportEntry> entries;
    };

    FileCleaner();
    ~FileCleaner();

    State currentState();
    QSet<ScanCategory> targetCategories();

    bool scan();
    bool scanByCategory(const QSet<ScanCategory> &categories);
    ScanReportType getScanReport();

    bool clean();
    bool cleanByScanReport(const ScanReportType& report);
    CleaningReportType getCleaningReport();

    void stop();

signals:
    void scanStaged(FileCleaner::ScanCategory category);
    void scanStageFinished(FileCleaner::ReportEntry finishedItem);
    void scanFinished();
    void cleaningStaged(QString ID);
    void cleaningStageFinished(FileCleaner::ReportEntry finishedItem);
    void cleaningFinished();
};

#endif // FILECLEANER_H
