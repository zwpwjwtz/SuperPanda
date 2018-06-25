#ifndef BOOTUTILS_H
#define BOOTUTILS_H

#include <QObject>
#include "../Interfaces/exelauncher.h"


class BootUtils : public QObject
{
    Q_OBJECT

public:
    BootUtils();

    bool updateBootMenu(QString configFilePath = "");

signals:
    void commandFinished(bool successful);

private:
    ExeLauncher exeFile;

private slots:
    void onExeFinished(ExeLauncher::ExecErrorCode errCode);
};

#endif // BOOTUTILS_H
