#include <QProcess>
#include "screenutils.h"

#define SPANDA_SCREEN_EXEC_CVT "cvt"
#define SPANDA_SCREEN_EXEC_XRANDR "xrandr"
#define SPANDA_SCREEN_EXEC_XRANDR_LISTMONITOR "--listmonitors"
#define SPANDA_SCREEN_EXEC_XRANDR_NEWMODE "--newmode"
#define SPANDA_SCREEN_EXEC_XRANDR_ADDMODE "--addmode"
#define SPANDA_SCREEN_EXEC_XRANDR_OUTPUT "--output"
#define SPANDA_SCREEN_EXEC_XRANDR_MODE "--mode"
#define SPANDA_SCREEN_EXEC_XGAMMA "xgamma"
#define SPANDA_SCREEN_EXEC_XGAMMA_SCREEN "-s"
#define SPANDA_SCREEN_EXEC_XGAMMA_RED "-rgamma"
#define SPANDA_SCREEN_EXEC_XGAMMA_GREEN "-ggamma"
#define SPANDA_SCREEN_EXEC_XGAMMA_BLUE "-bgamma"


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

QList<double> ScreenUtils::currentGamma(QString monitor)
{
    QProcess exe;
    if (monitor.isEmpty())
        exe.start(SPANDA_SCREEN_EXEC_XGAMMA);
    else
        exe.start(QString("%1 %2 %3")
                         .arg(SPANDA_SCREEN_EXEC_XGAMMA)
                         .arg(SPANDA_SCREEN_EXEC_XGAMMA_SCREEN)
                         .arg(monitor));
    exe.waitForFinished();

    // Assuming the first line of the output looks like this:
    // -> Red  0.950, Green  0.900, Blue  0.850
    // Note: The output of xgamma command is redirected to STDERROR!
    QList<QByteArray> output = exe.readAllStandardError().split('\n');
    QList<double> gammaComponent;
    QList<QByteArray> tempLine;
    int i, p;
    if (output.count() > 0)
    {
        tempLine = output[0].replace("-> ", "").split(',');
        for (i=0; i<tempLine.count(); i++)
        {
            p = tempLine[i].lastIndexOf(' ') + 1;
            if (p < tempLine[i].length())
                gammaComponent.push_back(tempLine[i].mid(p).toDouble());
        }
    }
    return gammaComponent;
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

bool ScreenUtils::setGamma(QString monitor,
                           double red, double green, double blue)
{
    QList<QString> args;
    if (!monitor.isEmpty())
    {
        args.push_back(SPANDA_SCREEN_EXEC_XGAMMA_SCREEN);
        args.push_back(monitor);
    }
    args.push_back(SPANDA_SCREEN_EXEC_XGAMMA_RED);
    args.push_back(QString::number(red));
    args.push_back(SPANDA_SCREEN_EXEC_XGAMMA_GREEN);
    args.push_back(QString::number(green));
    args.push_back(SPANDA_SCREEN_EXEC_XGAMMA_BLUE);
    args.push_back(QString::number(blue));

    QProcess exe;
    exe.start(SPANDA_SCREEN_EXEC_XGAMMA, args);
    exe.waitForFinished();
    return (exe.exitCode() == 0);
}
