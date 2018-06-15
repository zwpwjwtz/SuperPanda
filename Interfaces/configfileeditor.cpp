#include <QTemporaryFile>
#include "configfileeditor.h"

#define SPANDA_IFACE_CONFEDIT_BUFFER_LEN 1024
#define SPANDA_IFACE_CONFEDIT_BACKUP_MAX 10


ConfigFileEditor::ConfigFileEditor()
{

}

bool ConfigFileEditor::fileExists(const QString& fileName)
{
    QFile file(fileName);
    return file.exists();
}

ConfigFileEditor::FileErrorCode
ConfigFileEditor::exists(const QString& fileName,
                         const QString& search,
                         bool& exist)
{
    QFile file(fileName);
    if (!file.exists())
        return FileNotFound;
    if (!file.open(QFile::ReadOnly))
        return NoPermission;

    QByteArray searchBytes(search.toUtf8());
    QByteArray buffer;
    exist = false;
    while (!file.atEnd())
    {
        buffer.append(file.read(SPANDA_IFACE_CONFEDIT_BUFFER_LEN));
        if (buffer.contains(searchBytes))
        {
            exist = true;
            break;
        }
        else
            buffer.remove(0, buffer.length() - searchBytes.length());
    }
    return FileOk;
}

ConfigFileEditor::FileErrorCode
ConfigFileEditor::findLine(const QString& fileName,
                           const QString& search,
                           QString& matchedLine)
{
    QFile file(fileName);
    if (!file.exists())
        return FileNotFound;
    if (!file.open(QFile::ReadOnly))
        return NoPermission;

    QByteArray searchBytes(search.toUtf8());
    QByteArray buffer;
    bool found = false;
    matchedLine.clear();
    while (!file.atEnd())
    {
        buffer = file.readLine();
        if (buffer.contains(searchBytes))
        {
            found = true;
            break;
        }
    }
    if (found)
        matchedLine = buffer;
    return FileOk;
}

ConfigFileEditor::FileErrorCode
ConfigFileEditor::replace(const QString& fileName,
                          const QString& search,
                          const QString& replace)
{
    QFile file(fileName);
    if (!file.exists())
        return FileNotFound;
    if (!file.open(QFile::ReadOnly))
        return NoPermission;

    int pos = -1;
    QByteArray searchBytes(search.toUtf8());
    QByteArray buffer, readBuffer;
    while (!file.atEnd())
    {
        readBuffer = file.read(SPANDA_IFACE_CONFEDIT_BUFFER_LEN);
        buffer.append(readBuffer);
        if (buffer.contains(searchBytes))
        {
            pos = file.pos() - buffer.length() + buffer.indexOf(searchBytes);
            break;
        }
        else
            buffer.remove(0, buffer.length() - searchBytes.length());
    }

    if (pos >= 0)
    {
        // Create a temporary file of the content to be modified
        QTemporaryFile tempFile;
        if (!tempFile.open())
            return NoPermission;
        file.seek(0);
        tempFile.write(file.readAll());
        file.close();

        // Modify the searched content with replacing content
        file.setFileName(fileName);
        file.open(QFile::ReadWrite);
        if (!file.isWritable())
            return NoPermission;
        file.seek(pos);
        file.write(replace.toUtf8());

        // Write the rest part of file according to
        // the content of the temp file
        tempFile.seek(pos + search.length());
        file.write(tempFile.readAll());
    }
    file.close();
    return FileOk;
}

ConfigFileEditor::FileErrorCode
ConfigFileEditor::regexpReplaceLine(const QString& fileName,
                                    const QString& search,
                                    const QString& regExp,
                                    const QString& replace)
{
    QString oldConfigLine, newConfigLine;
    findLine(fileName, search, oldConfigLine);
    if (oldConfigLine.isEmpty())
        newConfigLine = search;
    else
        newConfigLine = oldConfigLine;
    newConfigLine.replace(QRegExp(regExp), replace);
    return ConfigFileEditor::replace(fileName, oldConfigLine, newConfigLine);
}

ConfigFileEditor::FileErrorCode ConfigFileEditor::backupFile(const QString &origin,
                                                             QString &destination)
{
    QFile file(origin);
    QString destName;
    int backupCount = 1;

    if (!file.exists())
        return FileNotFound;

    if (destName.isEmpty())
    {
        while (backupCount <= SPANDA_IFACE_CONFEDIT_BACKUP_MAX)
        {
            destName = QString(origin)
                       .append(".bak.")
                       .append(QString::number(backupCount));
            file.setFileName(destName);
            if (file.exists())
                backupCount++;
            else
                break;
        }
        if (file.exists())
        {
            // Shift backup file number
            // i.e. delete 1, 2=>1, 3=>2, ..., MAX=>MAX-1
            // Then we can use MAX for the number of new backup file
            if (!QFile::remove(QString(origin)
                          .append(".bak.")
                          .append(QString::number(1))))
                return NoPermission;

            for (int i=2; i<=SPANDA_IFACE_CONFEDIT_BACKUP_MAX; i++)
            {
                file.setFileName(QString(origin)
                                 .append(".bak.")
                                 .append(QString::number(i)));
                if (!file.exists())
                    continue;
                if (!file.rename(QString(origin)
                                 .append(".bak.")
                                 .append(QString::number(i - 1))))
                    return NoPermission;
            }
        }
    }

    file.setFileName(origin);
    if (!QFile::copy(origin, destName))
        return NoPermission;
    else
    {
        destination = destName;
        return FileOk;
    }
}
