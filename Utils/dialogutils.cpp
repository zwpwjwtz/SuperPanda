#include <QMessageBox>
#include "dialogutils.h"


DialogUtils::DialogUtils()
{

}

void DialogUtils::warnMissingFile(QString fileName, bool aborted)
{
    if (aborted)
    {
        QMessageBox::critical(nullptr, tr("Missing file"),
                              QString(
                                  tr("Cannot continue due to a missing file: "
                                      "\n%1")
                                     ).arg(fileName));
    }
    else
    {
        QMessageBox::warning(nullptr, tr("Missing file"),
                             QString(
                                 tr("File %1 does not exists. We will try to "
                                     "create it if possible.")
                                    ).arg(fileName));
    }
}

void DialogUtils::warnPermission(QString objectName)
{
    QMessageBox::critical(nullptr, tr("Permission denied"),
                          QString(
                              tr("Cannot continue due to denied access to "
                                  "\n%1")
                                 ).arg(objectName));
}

void DialogUtils::warnExecPermission(QString objectName)
{
    QMessageBox::critical(nullptr, tr("Permission denied"),
                          QString(
                              tr("Cannot execute %1.\n"
                                  "Please make sure that you have the "
                                  "right permission to do it.")
                                 ).arg(objectName));
}

void DialogUtils::warnInsufficientSpace(QString path, qint64 requiredSpace)
{
    requiredSpace /= 1024 * 1024; // Use MiB as readable unit
    QMessageBox::critical(nullptr, tr("Insufficient Space"),
                          QString(tr("One or more operation failed due to "
                                     "insufficient space of directory %1.\n"
                                     "Please make sure that it has at least "
                                     "%2 MB free space."))
                                 .arg(path)
                                 .arg(QString::number(requiredSpace)));
}
