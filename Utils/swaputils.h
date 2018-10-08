#ifndef SWAPUTILS_H
#define SWAPUTILS_H

#include "../Interfaces/exelauncher.h"


class SwapUtils : public QObject
{
    Q_OBJECT
public:
    SwapUtils();

    static bool isSwapFile(QString fileName);
    bool makeSwapFile(QString fileName, qint64 size);
    bool removeSwapFile(QString fileName);

    bool turnOnSwap(QString device);
    bool turnOffSwap(QString device);

signals:
    void finishedMakingSwapfile();
    void finishedRemovingSwapfile();
    void finishedTurnOnSwap();
    void finishedTurnOffSwap();
    void operationFailed();

private:
    ExeLauncher exe;
    QString currentDevice;
    int deviceSize;
    int currentOperation;

    bool createSwapFile(QString fileName, qint64 size);
    bool setupSwapFile(QString fileName);

private slots:
    void onExeFinished(ExeLauncher::ExecErrorCode errCode);
};

#endif // SWAPUTILS_H
