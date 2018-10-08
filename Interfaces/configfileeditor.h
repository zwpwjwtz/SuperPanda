#ifndef CONFIGFILEEDITOR_H
#define CONFIGFILEEDITOR_H

#include <QString>


class ConfigFileEditor
{
public:
    enum FileErrorCode
    {
        FileOk = 0,
        FileNotFound = 1,
        NoPermission = 2,
        CannotBackup = 3,
        UnknownError = 255
    };

    ConfigFileEditor();

    static bool fileExists(const QString& fileName);
    static qint64 fileSize(const QString& fileName);
    static FileErrorCode backupFile(const QString& origin,
                                    QString& destination);
    static FileErrorCode deleteFile(const QString& fileName);

    static FileErrorCode exists(const QString& fileName,
                                const QString& search,
                                bool& exist);
    static FileErrorCode findLine(const QString& fileName,
                                  const QString& search,
                                  QString& matchedLine);
    static FileErrorCode replace(const QString& fileName,
                                 const QString& search,
                                 const QString& replace);
    static FileErrorCode replaceLine(const QString& fileName,
                                     const QString& search,
                                     const QString& replace,
                                     bool appendIfNotFound = true);
    static FileErrorCode regexpReplaceLine(const QString& fileName,
                                           const QString& search,
                                           const QString& expression,
                                           const QString& replace);
    static FileErrorCode regexpWriteLine(const QString& fileName,
                                         const QString& search,
                                         const QString& regExp,
                                         QString replace,
                                         bool replaceIfExists = true);
    static FileErrorCode append(const QString& fileName,
                                const QString& content);

private:
    static QString expandFileName(const QString& fileName);
};

#endif // CONFIGFILEEDITOR_H
