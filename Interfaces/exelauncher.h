#ifndef EXELAUNCHER_H
#define EXELAUNCHER_H

#include <QObject>
#include <QProcess>


class ExeLauncher : QObject
{
    Q_OBJECT

public:
    enum class ExecErrorCode
    {
        ExecOk = 0,
        FileNotFound = 1,
        NoPermission = 2,
        UnknownError = 255
    };

    ExeLauncher();

    ExecErrorCode runFile(QString filePath);
    QString getOutput();
    QString getExitCode();

signals:
    void finished(int exitCode);

private:
    QProcess process;
};

#endif // EXELAUNCHER_H
