#include <QMessageBox>
#include "bootutils.h"
#include "dialogutils.h"
#include "../Interfaces/configfileeditor.h"

#define SPANDA_MGCKF_EXEC_GRUB_UPDATE "/usr/sbin/grub2-mkconfig"
#define SPANDA_MGCKF_EXEC_GRUB_UPDATE2 "/usr/sbin/grub-mkconfig"

#define SPANDA_MGCKF_FILE_GRUB_CONFIG "/boot/grub2/grub.cfg"
#define SPANDA_MGCKF_FILE_GRUB_CONFIG2 "/boot/grub/grub.cfg"


BootUtils::BootUtils()
{
    connect(&exeFile,
            SIGNAL(finished(ExeLauncher::ExecErrorCode)),
            this,
            SLOT(onExeFinished(ExeLauncher::ExecErrorCode)));
}

bool BootUtils::updateBootMenu(QString configFilePath)
{
    // Update grub config file using grub-mkconfig

    QList<QString> tempStringList;
    tempStringList.append("-o");

    if (configFilePath.isEmpty())
    {
        if (ConfigFileEditor::fileExists(SPANDA_MGCKF_FILE_GRUB_CONFIG))
            tempStringList.append(SPANDA_MGCKF_FILE_GRUB_CONFIG);
        else
            tempStringList.append(SPANDA_MGCKF_FILE_GRUB_CONFIG2);
    }
    else
    {
        if (!ConfigFileEditor::fileExists(configFilePath))
            return false;
    }

    QString fileName;
    if (exeFile.fileExecutable(SPANDA_MGCKF_EXEC_GRUB_UPDATE))
        fileName = SPANDA_MGCKF_EXEC_GRUB_UPDATE;
    else
        fileName = SPANDA_MGCKF_EXEC_GRUB_UPDATE2;

    return (exeFile.runFile(fileName, tempStringList) == ExeLauncher::ExecOk);
}

void BootUtils::onExeFinished(ExeLauncher::ExecErrorCode errCode)
{
    switch (errCode)
    {
        case ExeLauncher::ExecOk:
            emit commandFinished(true);
            return;
        case ExeLauncher::FileNotFound:
            DialogUtils::warnMissingFile(exeFile.getExeFilePath(), true);
            break;
        case ExeLauncher::NoPermission:
            DialogUtils::warnExecPermission(exeFile.getExeFilePath());
            break;
        case ExeLauncher::Crashed:
        case ExeLauncher::UnknownError:
        default:
            QMessageBox::critical(nullptr, "Unknown error occured",
                                  QString("Magic Panda encountered an exception "
                                          "when trying to execute program \n%1\n"
                                          "Exit code: %2\n")
                                         .arg(exeFile.getCommand())
                                         .arg(exeFile.getExitCode()));
    }
    emit commandFinished(false);
}
