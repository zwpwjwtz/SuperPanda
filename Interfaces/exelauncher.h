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
                             bool synchronous = false);
    QString getCommand();
    QString getExeFilePath();
    QByteArray getOutput();
    int getExitCode();
    ExecErrorCode getErrCode();
    static bool fileExecutable(const QString& filePath);

signals:
    void finished(ExeLauncher::ExecErrorCode errCode);

private:
    QString execPath;
    QString command;
    QProcess process;
    QByteArray output;

private slots:
    void onProcessError(QProcess::ProcessError errCode);
    void onProcessFinished(int exitCode);
};

#endif // EXELAUNCHER_H
