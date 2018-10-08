#ifndef DISKUTILS_H
#define DISKUTILS_H

#include <QtGlobal>

class DiskUtils
{
public:
    DiskUtils();

    static QString getUUIDByBlock(QString blockName);
    static qint64 getFreeSpace(QString path);
};

#endif // DISKUTILS_H
