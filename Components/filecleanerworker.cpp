#include "filecleanerworker.h"


FileCleanerWorker::FileCleanerWorker(QObject *parent) : QObject(parent)
{
    _requiredStop = false;
}

void FileCleanerWorker::startScanning(QSet<FileCleaner::ScanCategory> taskList)
{
    _requiredStop = false;

    QSet<FileCleaner::ScanCategory>::const_iterator i;
    for (i=taskList.constBegin(); i!=taskList.constEnd(); i++)
    {
        _mutexRequiredStop.lock();
        if (_requiredStop)
            return;
        _mutexRequiredStop.unlock();

        emit scanningStageStarted(*i);
        switch (*i)
        {
            case FileCleaner::ScanAppCache:

                break;
            case FileCleaner::ScanDownloadPackage:

                break;
            case FileCleaner::ScanUnusedPackageConf:

                break;
            default:;
        }
        emit scanningStageFinished(*i, 0, 0);
    }
    emit scanningFinished();
}

void FileCleanerWorker::startCleaning(FileCleaner::ScanReportType taskList)
{
    _requiredStop = false;

    for (int i=0; i<taskList.entries.count(); i++)
    {
        _mutexRequiredStop.lock();
        if (_requiredStop)
            return;
        _mutexRequiredStop.unlock();

        switch (taskList.entries[i].category)
        {
            case FileCleaner::ScanAppCache:

                break;
            case FileCleaner::ScanDownloadPackage:

                break;
            case FileCleaner::ScanUnusedPackageConf:

                break;
            default:;
        }
    }
    emit cleaningFinished();
}

void FileCleanerWorker::stop()
{
    QMutexLocker locker(&_mutexRequiredStop);
    _requiredStop = true;
}
