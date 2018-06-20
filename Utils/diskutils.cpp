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
