#ifndef FILECLEANER_P_H
#define FILECLEANER_P_H

#include <QDateTime>
#include "filecleaner.h"
#include "filecleanerworker.h"


class FileCleanerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FileCleaner)
protected:
    FileCleaner* q_ptr;

public:
    typedef FileCleaner::State State;
    typedef FileCleaner::ScanCategory ScanCategory;

    QThread workerThread;
    FileCleanerWorker worker;

    State currentState;
    QDateTime startTime;
    QSet<ScanCategory> targetCateogries;

    FileCleaner::ScanReportType scanReport;
    FileCleaner::CleaningReportType cleanReport;

    FileCleanerPrivate(FileCleaner* parent);
    ~FileCleanerPrivate();

signals:
    void startScanningWorker(QSet<FileCleaner::ScanCategory> categories);
    void startCleaningWorker(FileCleaner::ScanReportType taskList);

protected slots:
    void workerScanStageStarted(FileCleaner::ScanCategory stage);
    void workerScanStageFinished(FileCleaner::ScanCategory stage,
                                 int itemCount,
                                 qint64 cleanableSize);
    void workerCleaningStageStarted(QString ID);
    void workerCleaningStageFinished(FileCleaner::ScanCategory stage,
                                     QString ID,
                                     qint64 cleanableSize);
    void workerFinished();
};

#endif // FILECLEANER_P_H
