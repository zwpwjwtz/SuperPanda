#ifndef MagicKonfug_H
#define MagicKonfug_H

#include <QMainWindow>
#include "Interfaces/configfileeditor.h"
#include "Utils/bootutils.h"
#include "Utils/environment.h"


class QAbstractButton;
class EnvironmentWidget;

namespace Ui {
class MagicKonfug;
}

class MagicKonfug : public QMainWindow
{
    Q_OBJECT

public:
    explicit MagicKonfug(QWidget *parent = 0);
    ~MagicKonfug();

protected:
    void changeEvent(QEvent* event);
    void closeEvent(QCloseEvent* event);
    void showEvent(QShowEvent* event);

private:
    enum class EnvEditMode
    {
        NotEditting = 0,
        SystemScope = 1,
        UserScope = 2,
        Disabled = 255
    };

    Ui::MagicKonfug *ui;
    EnvironmentWidget* envEditor;
    ConfigFileEditor configFile;
    BootUtils bootConfig;
    QList<Utils::EnvironmentItem> envVarChanges;
    static const int pageGroupCount = 6;
    static const int configEntryCount = 12;
    EnvEditMode currentEnvEditMode;
    bool configPageMoidified[pageGroupCount];
    bool configMoidified[configEntryCount];
    bool needUpdatingBoot;

    void loadConfig();
    bool applyConfig(int configIndex);
    void setConfigModified(int configIndex, bool modified = true);
    void setConfigPageModified(int pageIndex, bool modified = true);
    void setWidgetDisabled(QWidget* widget);
    void showEnvEditor(bool systemScope = false);
    void showStatusPage(bool pageVisible, QString text = QString());
    static void destroyWidget(QWidget* widget);
    static bool testConfigFileError(ConfigFileEditor::FileErrorCode errCode,
                                    const QString& fileName,
                                    const bool aborted = true);
    static bool writeEnvConfigFile(QString fileName,
                                   QList<Utils::EnvironmentItem> changes);
    static int composeKeyStringToIndex(const QString& str);
    static QString composeKeyIndexToString(int index);
    static int bootResolutionStringToIndex(const QString& str);
    static QString bootResolutionIndexToString(int index);

private slots:
    void onCommandFinished(bool successful);
    void onEnvEditorClosing();

    void on_listWidget_clicked(const QModelIndex &index);
    void on_buttonAbout_clicked();
    void on_buttonExit_clicked();
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_groupPage_currentChanged(int arg1);
    void on_textTimeoutSrvStart_valueChanged(int arg1);
    void on_textTimeoutShutdown_valueChanged(int arg1);
    void on_checkTurboFreq_toggled(bool checked);
    void on_comboKeySequence_currentIndexChanged(int index);
    void on_comboDiskType_currentIndexChanged(const QString &arg1);
    void on_textTimeoutBoot_valueChanged(int arg1);
    void on_comboBootResolution_currentIndexChanged(int index);
    void on_checkIWiFi80211n_toggled(bool checked);
    void on_buttonEditSysEnv_clicked();
    void on_buttonEditUserEnv_clicked();
    void on_textWindowScaling_valueChanged(int arg1);
    void on_textWindowTextScaling_valueChanged(double arg1);
};

#endif // MagicKonfug_H
