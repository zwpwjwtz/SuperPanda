#include "diskutils.h"
#include "../Interfaces/exelauncher.h"


DiskUtils::DiskUtils()
{

}

QString DiskUtils::getUUIDByBlock(QString blockName)
{
    ExeLauncher exe;
    exe.runCommand(QString("blkid | grep \"^%1\"").arg(blockName), true);

    QString result;
    QList<QByteArray> output(exe.getOutput().split(' '));
    if (output.count() > 2)
    {
        // Use the third field as UUID of the specified block device
        // Assuming field format: UUID="XXX"
        result = output[2];
        result = result.mid(6, result.length() - 7);
    }
    return result;
}

qint64 DiskUtils::getFreeSpace(QString path)
{
    ExeLauncher exe;
    exe.runCommand(QString("df -B 1 %1").arg(path), true);

    qint64 result = 0;
    QList<QByteArray> output(exe.getOutput().split('\n'));
    if (output.count() > 2)
    {
        // The second line should contain space info of given device
        // Assuming the fields follow the style:
        // /dev/sda2  1000000000 200000000 800000000  20% /home
        QList<QString> fieldList = QString::fromUtf8(output[1])
                                        .split(' ', QString::SkipEmptyParts);
        if (fieldList.count() > 4)
            result = fieldList[3].toLongLong();
    }
    return result;
}
