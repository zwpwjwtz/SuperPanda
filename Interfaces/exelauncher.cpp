#include <QFileInfo>
#include "exelauncher.h"

#define SPANDA_EXEC_SHELL_DEFAULT "/bin/bash"
#define SPANDA_EXEC_SHELL_ARG_EXEC "-c"

ExeLauncher::ExeLauncher()
{
    connect(&process,
            SIGNAL(error(QProcess::ProcessError)),
            this,
            SLOT(onProcessError(QProcess::ProcessError)));
    connect(&process,
            SIGNAL(finished(int)),
            this,
            SLOT(onProcessFinished(int)));
}

ExeLauncher::ExecErrorCode
ExeLauncher::runFile(const QString& filePath,
                     const QList<QString>& arguments,
                     bool synchronous)
{
    // Save command information
    command = filePath;
    command.append(' ').append(arguments.join(' '));
    execPath = filePath;

    ExecErrorCode errCode =  ExecOk;
    process.start(filePath, arguments);
    if (synchronous)
    {
        process.waitForStarted();
        process.waitForFinished();
        output = process.readAllStandardOutput();
        errCode = getErrCode();
        if (errCode != ExecOk)
            output.append(process.readAllStandardError());
    }
    return errCode;
}

ExeLauncher::ExecErrorCode
ExeLauncher::runCommand(const QString& command,
                        const QList<QString>& arguments,
                        bool synchronous)
{
    QStringList args(arguments.join(" "));
    args.push_front(command);
    args.push_front(SPANDA_EXEC_SHELL_ARG_EXEC);
    return runFile(SPANDA_EXEC_SHELL_DEFAULT, args, synchronous);
}

QString ExeLauncher::getCommand()
{
    return command;
}

QString ExeLauncher::getExeFilePath()
{
    return execPath;
}

QByteArray ExeLauncher::getOutput()
{
    return output;
}

int ExeLauncher::getExitCode()
{
    return process.exitCode();
}

ExeLauncher::ExecErrorCode ExeLauncher::getErrCode()
{
    if (process.exitStatus() == QProcess::CrashExit)
        return Crashed;
    switch (process.exitCode())
    {
        case 0:
            return ExecOk;
        case 1:
            return NoPermission;
        case 126:
            return NoPermission;
        case 127:
            return FileNotFound;
        default:
            return UnknownError;
    }
}

bool ExeLauncher::fileExecutable(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isExecutable();
}

void ExeLauncher::onProcessError(QProcess::ProcessError errCode)
{
    output.clear();
    switch (errCode)
    {
        case QProcess::Crashed:
            emit finished(Crashed);
            break;
        case QProcess::FailedToStart:
            emit finished(NoPermission);
            break;
        default:
            emit finished(UnknownError);
    }
}

void ExeLauncher::onProcessFinished(int exitCode)
{
    Q_UNUSED(exitCode)
    output = process.readAllStandardOutput();
    ExecErrorCode errCode = getErrCode();
    if (errCode != ExecOk)
        output.append(process.readAllStandardError());
    emit finished(errCode);
}
