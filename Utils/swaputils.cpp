#include <QFileInfo>
#include <QMessageBox>

#include "swaputils.h"
#include "dialogutils.h"

#define SPANDA_SWAPUTILS_FILE_TYPE_SWAPFILE "swap file"

#define SPANDA_SWAPUTILS_OP_NONE 0
#define SPANDA_SWAPUTILS_OP_PRE_MAKE_SWAPFILE 1
#define SPANDA_SWAPUTILS_OP_MAKE_SWAPFILE 2
#define SPANDA_SWAPUTILS_OP_PRE_REMOVE_SWAPFILE 3
#define SPANDA_SWAPUTILS_OP_REMOVE_SWAPFILE 4
#define SPANDA_SWAPUTILS_OP_SETUP_SWAP 5
#define SPANDA_SWAPUTILS_OP_TURNON_SWAP 6
#define SPANDA_SWAPUTILS_OP_TURNOFF_SWAP 7


SwapUtils::SwapUtils()
{
    deviceSize = 0;
    currentOperation = SPANDA_SWAPUTILS_OP_NONE;
    connect(&exe,
            SIGNAL(finished(ExeLauncher::ExecErrorCode)),
            this,
            SLOT(onExeFinished(ExeLauncher::ExecErrorCode)));
}

bool SwapUtils::isSwapFile(QString fileName)
{
    ExeLauncher exe;
    exe.runCommand(QString("file %1").arg(fileName), true);
    return exe.getOutput().contains(SPANDA_SWAPUTILS_FILE_TYPE_SWAPFILE);
}

bool SwapUtils::makeSwapFile(QString fileName, qint64 size)
{
    QFileInfo swapfile(fileName);
    if (swapfile.exists())
    {
        // Check if the file belongs to root
        if (swapfile.ownerId() != 0)
            return false;

        // Check if it is not a swap file
        if (!isSwapFile(fileName))
            return false;

        turnOffSwap(fileName);

        // NOTE: the following assignment may fail if the callback
        // triggered by exe.runCommand() called in turnOffSwap()
        // comes before the returning of turnOffSwap()
        currentOperation = SPANDA_SWAPUTILS_OP_PRE_MAKE_SWAPFILE;
        deviceSize = size;
    }
    else
        createSwapFile(fileName, size);
    return true;
}

bool SwapUtils::removeSwapFile(QString fileName)
{
    QFile swapfile(fileName);
    if (swapfile.exists())
    {
        if (!isSwapFile(fileName))
            return false;
    }

    currentDevice = fileName;
    currentOperation = SPANDA_SWAPUTILS_OP_PRE_REMOVE_SWAPFILE;
    exe.runCommand(QString("swapoff %1").arg(fileName));
    return true;
}

bool SwapUtils::turnOnSwap(QString device)
{
    currentDevice = device;
    currentOperation = SPANDA_SWAPUTILS_OP_TURNON_SWAP;
    exe.runCommand(QString("swapon %1").arg(device));
    return true;
}

bool SwapUtils::turnOffSwap(QString device)
{
    currentDevice = device;
    currentOperation = SPANDA_SWAPUTILS_OP_TURNOFF_SWAP;
    exe.runCommand(QString("swapoff %1").arg(device));
    return true;
}

bool SwapUtils::createSwapFile(QString fileName, qint64 size)
{
    currentDevice = fileName;
    currentOperation = SPANDA_SWAPUTILS_OP_MAKE_SWAPFILE;
    exe.runCommand(QString("dd if=/dev/zero of=%1 bs=1M count=%2")
                          .arg(fileName)
                          .arg(QString::number(size / 1024 / 1024)));
    return true;
}

bool SwapUtils::setupSwapFile(QString fileName)
{
    // Initialize the swap filesystem
    currentDevice = fileName;
    currentOperation = SPANDA_SWAPUTILS_OP_SETUP_SWAP;
    exe.runCommand(QString("chmod 600 %1").arg(fileName), true);
    exe.runCommand(QString("mkswap %1").arg(fileName), true);
    return true;
}

void SwapUtils::onExeFinished(ExeLauncher::ExecErrorCode errCode)
{
    switch (errCode)
    {
        case ExeLauncher::ExecOk:
        {
        switch (currentOperation)
        {
            case SPANDA_SWAPUTILS_OP_PRE_MAKE_SWAPFILE:
                createSwapFile(currentDevice, deviceSize);
                break;
            case SPANDA_SWAPUTILS_OP_MAKE_SWAPFILE:
                if (setupSwapFile(currentDevice))
                    emit finishedMakingSwapfile();
                else
                    emit operationFailed();
                break;
            case SPANDA_SWAPUTILS_OP_PRE_REMOVE_SWAPFILE:
                currentOperation = SPANDA_SWAPUTILS_OP_REMOVE_SWAPFILE;
                if (exe.runCommand(QString("rm -f %1").arg(currentDevice), true)
                        == ExeLauncher::ExecOk)
                    emit finishedRemovingSwapfile();
                else
                    emit operationFailed();
                break;
            case SPANDA_SWAPUTILS_OP_TURNON_SWAP:
                emit finishedTurnOnSwap();
                break;
            case SPANDA_SWAPUTILS_OP_TURNOFF_SWAP:
                emit finishedTurnOffSwap();
                break;
            default:;
        }
        break;
        }
        case ExeLauncher::FileNotFound:
            DialogUtils::warnMissingFile(exe.getExeFilePath(), true);
            break;
        case ExeLauncher::NoPermission:
            DialogUtils::warnExecPermission(exe.getExeFilePath());
            break;
        case ExeLauncher::Crashed:
        case ExeLauncher::UnknownError:
        default:
            emit operationFailed();
    }
}
