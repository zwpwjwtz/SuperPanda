#include <QProcess>
#include "screenutils.h"

#define SPANDA_SCREEN_EXEC_CVT "cvt"
#define SPANDA_SCREEN_EXEC_XRANDR "xrandr"
#define SPANDA_SCREEN_EXEC_XRANDR_LISTMONITOR "--listmonitors"
#define SPANDA_SCREEN_EXEC_XRANDR_NEWMODE "--newmode"
#define SPANDA_SCREEN_EXEC_XRANDR_ADDMODE "--addmode"
#define SPANDA_SCREEN_EXEC_XRANDR_OUTPUT "--output"
#define SPANDA_SCREEN_EXEC_XRANDR_MODE "--mode"


ScreenUtils::ScreenUtils()
{
}

QList<QString> ScreenUtils::getMonitors()
{
    QProcess exe;
    exe.start(QString("%1 %2")
                     .arg(SPANDA_SCREEN_EXEC_XRANDR)
                     .arg(SPANDA_SCREEN_EXEC_XRANDR_LISTMONITOR));
    exe.waitForFinished();

    // Assuming the first line is the total number of monitors
    // and the following lines give the details
    QList<QByteArray> output = exe.readAllStandardOutput().split('\n');
    int monitorCount = output[0].mid(output[0].indexOf(": ") + 2).toInt();
    QList<QString> monitorList;
    QList<QByteArray> tempLine;
    for (int i=0; i<monitorCount; i++)
    {
        if (i + 1 >= output.count())
            break;

        tempLine = output[i + 1].split(' ');
        if (tempLine.count() > 0)
            monitorList.push_back(QString::fromUtf8(tempLine.last()));
    }

    return monitorList;
}

QString ScreenUtils::currentMonitor()
{
    QList<QString> monitorList(getMonitors());
    if (monitorList.count() > 0)
        return monitorList[0];
    else
        return "";
}

QSize ScreenUtils::currentResolution(QString monitor)
{
    QProcess exe;
    exe.start(QString("%1 %2")
                     .arg(SPANDA_SCREEN_EXEC_XRANDR)
                     .arg(SPANDA_SCREEN_EXEC_XRANDR_LISTMONITOR));
    exe.waitForFinished();

    // Same output as getMonitors()
    QList<QByteArray> output = exe.readAllStandardOutput().split('\n');
    int monitorCount = output[0].mid(output[0].indexOf(": ") + 2).toInt();
    QList<QString> tempLine;
    QSize resolution;
    for (int i=0; i<monitorCount; i++)
    {
        if (i + 1 >= output.count())
            break;

        // Assuming each line follows the style below:
        //  0: +*eDP1 1024/270x768/160+0+0  eDP1
        //  No.       Width    Height      Monitor
        tempLine = QString::fromUtf8(output[i + 1])
                                    .split(' ', QString::SkipEmptyParts);
        if (tempLine.count() >= 4)
        {
            if (!monitor.isEmpty() && tempLine[3] != monitor)
                continue;
            tempLine = tempLine[2].split('x');
            resolution.setWidth(tempLine[0].left(tempLine[0].indexOf('/'))
                                           .toInt());
            resolution.setHeight(tempLine[1].left(tempLine[0].indexOf('/'))
                                            .toInt());
            break;
        }
    }
    return resolution;
}

QString ScreenUtils::getModeLine(QSize resolution, int refreshRate)
{
    QString refreshRateString;
    if (refreshRate > 0)
        refreshRateString = QString::number(refreshRate);

    QProcess exe;
    exe.start(QString("%1 %2 %3 %4")
                     .arg(SPANDA_SCREEN_EXEC_CVT)
                     .arg(QString::number(resolution.width()))
                     .arg(QString::number(resolution.height()))
                     .arg(refreshRateString));
    exe.waitForFinished();

    QList<QByteArray> output = exe.readAll().split('\n');
    if (output.count() < 2)
        return "";
    else
        return QString::fromUtf8(output[1].replace("Modeline ", ""));
}

QString ScreenUtils::getModeName(QString modeLine)
{
    return modeLine.left(modeLine.indexOf(' '));
}

bool ScreenUtils::setMode(QString monitor,
                          QSize resolution,
                          int refreshRate)
{
    QString modeLine = getModeLine(resolution, refreshRate);
    QString modeName = modeLine.left(modeLine.indexOf(' '));
    if (modeName.isEmpty())
        return false;

    // Create a new mode
    QProcess exe;
    exe.start(QString("%1 %2 %3")
                     .arg(SPANDA_SCREEN_EXEC_XRANDR)
                     .arg(SPANDA_SCREEN_EXEC_XRANDR_NEWMODE)
                     .arg(modeLine));
    exe.waitForFinished();

    // Bind the mode to the given monitor
    if (!getMonitors().contains(monitor))
        return false;
    exe.start(QString("%1 %2 %3 %4")
                     .arg(SPANDA_SCREEN_EXEC_XRANDR)
                     .arg(SPANDA_SCREEN_EXEC_XRANDR_ADDMODE)
                     .arg(monitor)
                     .arg(modeName));
    exe.waitForFinished();
    if (exe.exitCode() != 0)
        return false;

    // Apply the mode
    exe.start(QString("%1 %2 %3 %4 %5")
                     .arg(SPANDA_SCREEN_EXEC_XRANDR)
                     .arg(SPANDA_SCREEN_EXEC_XRANDR_OUTPUT)
                     .arg(monitor)
                     .arg(SPANDA_SCREEN_EXEC_XRANDR_MODE)
                     .arg(modeName));
    exe.waitForFinished();
    return (exe.exitCode() == 0);
}
