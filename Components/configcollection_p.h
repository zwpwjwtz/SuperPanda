#ifndef CONFIGCOLLECTION_P_H
#define CONFIGCOLLECTION_P_H

#include <QObject>
#include <QMap>
#include "../Interfaces/configfileeditor.h"
#include "../Interfaces/exelauncher.h"
#include "../Utils/gsettingseditor.h"
#include "../Utils/bootutils.h"
#include "../Utils/swaputils.h"


class ConfigCollection;

class ConfigCollectionPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ConfigCollection)
protected:
    ConfigCollection* q_ptr;

public:
    QMap<int, QVariant> configList;
    QMap<int, bool> configModified;
    BootUtils bootConfig;
    SwapUtils swapConfig;
    ConfigFileEditor configFile;
    ExeLauncher exeFile;
    bool needResetScreen;
    bool needResetSwapFile;
    bool needUpdatingBoot;
    static const int MaxConfigEntry = 14;

    ConfigCollectionPrivate(ConfigCollection* parent);
    void doUpdating();
    static QString getEnvironmentVariable(QString key);
    static bool testConfigFileError(ConfigFileEditor::FileErrorCode errCode,
                                    const QString& fileName,
                                    const bool aborted = true);

protected slots:
    void onUpdatingBootFinished();
    void onFinishedMakingSwapfile();
    void onFinishedRemovingSwapfile();
    void onFinishedTurnOnSwap();
};

#endif // CONFIGCOLLECTION_P_H
