#ifndef DIALOGUTILS_H
#define DIALOGUTILS_H

#include <QCoreApplication>


class DialogUtils
{
    Q_DECLARE_TR_FUNCTIONS(DialogUtils)

public:
    DialogUtils();

    static void warnMissingFile(QString fileName, bool aborted = false);
    static void warnPermission(QString objectName);
    static void warnExecPermission(QString objectName);
    static void warnInsufficientSpace(QString path, qint64 requiredSpace);
};

#endif // DIALOGUTILS_H
