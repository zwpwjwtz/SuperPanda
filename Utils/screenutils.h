#ifndef SCREENUTILS_H
#define SCREENUTILS_H

#include <QSize>


class ScreenUtils
{
public:
    ScreenUtils();

    static QList<QString> getMonitors();
    static QString currentMonitor();
    static QSize currentResolution(QString monitor = "");

    static QString getModeLine(QSize resolution, int refreshRate = 0);
    static QString getModeName(QString modeLine);
    static bool setMode(QString monitor,
                        QSize resolution,
                        int refreshRate = 0);
};

#endif // SCREENUTILS_H
