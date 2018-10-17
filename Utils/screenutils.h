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
    static QList<double> currentGamma(QString monitor = "");

    static QString getModeLine(QSize resolution, int refreshRate = 0);
    static QString getModeName(QString modeLine);
    static bool setMode(QString monitor,
                        QSize resolution,
                        int refreshRate = 0);
    static bool setGamma(QString monitor,
                         double red, double green, double blue);
};

#endif // SCREENUTILS_H
