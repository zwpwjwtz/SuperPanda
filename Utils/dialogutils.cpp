#include <QMessageBox>
#include "dialogutils.h"


DialogUtils::DialogUtils()
{

}

void DialogUtils::warnMissingFile(QString fileName, bool aborted)
{
    if (aborted)
    {
        QMessageBox::critical(nullptr, "Missing file",
                              QString("Cannot continue due to a missing file: "
                                      "\n%1").arg(fileName));
    }
    else
    {
        QMessageBox::warning(nullptr, "Missing file",
                             QString("File %1 does not exists. We will try to "
                                     "create it if possible.").arg(fileName));
    }
}

void DialogUtils::warnPermission(QString objectName)
{
    QMessageBox::critical(nullptr, "Permission denied",
                          QString("Cannot continue due to denied access to "
                                  "\n%1").arg(objectName));
}

void DialogUtils::warnExecPermission(QString objectName)
{
    QMessageBox::critical(nullptr, "Permission denied",
                          QString("Cannot execute %1.\n"
                                  "Please make sure that you have the "
                                  "right permission to do it.")
                                 .arg(objectName));
}
