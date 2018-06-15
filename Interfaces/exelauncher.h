#ifndef EXELAUNCHER_H
#define EXELAUNCHER_H

#include <QObject>
#include <QProcess>


class ExeLauncher : public QObject
{
    Q_OBJECT

public:
    enum ExecErrorCode
    {
        ExecOk = 0,
        FileNotFound = 1,
        NoPermission = 2,
        Crashed = 3,
        UnknownError = 255
    };

    ExeLauncher();

    ExecErrorCode runFile(const QString& filePath,
                          const QList<QString>& arguments,
                          bool synchronous = false);
    ExecErrorCode runCommand(const QString& command,
                             const QList<QString>& arguments,
                             bool synchronous = false);
    QString getCommand();
    QString getExeFilePath();
    QByteArray getOutput();
    int getExitCode();

signals:
    void finished(ExeLauncher::ExecErrorCode errCode);

private:
    QString execPath;
    QString command;
    QProcess process;
    QByteArray output;
    ExecErrorCode getErrCode();

private slots:
    void onProcessError(QProcess::ProcessError errCode);
    void onProcessFinished(int exitCode);
};

#endif // EXELAUNCHER_H
