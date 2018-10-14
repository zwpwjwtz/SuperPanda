#ifndef FILECLEANERWORKER_H
#define FILECLEANERWORKER_H

#include <QMutex>
#include "filecleaner.h"


class FileCleanerWorker : public QObject
{
    Q_OBJECT

public:
    explicit FileCleanerWorker(QObject *parent = 0);

public slots:
    void startScanning(QSet<FileCleaner::ScanCategory> taskList);
    void startCleaning(FileCleaner::ScanReportType taskList);
    void stop();

signals:
    void scanningStageStarted(FileCleaner::ScanCategory stage);
    void scanningStageFinished(FileCleaner::ScanCategory stage,
                               int itemCount,
                               qint64 cleanableSize);
    void scanningFinished();
    void cleaningStageStarted(QString ID);
    void cleaningStageFinished(FileCleaner::ScanCategory stage,
                               QString ID,
                               qint64 cleanableSize);
    void cleaningFinished();

private:
    QMutex _mutexRequiredStop;
    bool _requiredStop;
};


Q_DECLARE_METATYPE(FileCleaner::ScanCategory)
Q_DECLARE_METATYPE(QSet<FileCleaner::ScanCategory>)
Q_DECLARE_METATYPE(FileCleaner::ScanReportType)

#endif // FILECLEANERWORKER_H
