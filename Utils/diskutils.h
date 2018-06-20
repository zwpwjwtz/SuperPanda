#ifndef DISKUTILS_H
#define DISKUTILS_H

#include <QtGlobal>

class DiskUtils
{
public:
    DiskUtils();

    static QString getUUIDByBlock(QString blockName);
};

#endif // DISKUTILS_H
