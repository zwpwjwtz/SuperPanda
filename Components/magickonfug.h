#ifndef MagicKonfug_H
#define MagicKonfug_H

#include <QMainWindow>
#include "Interfaces/configfileeditor.h"
#include "Interfaces/exelauncher.h"


class QAbstractButton;

namespace Ui {
class MagicKonfug;
}

class MagicKonfug : public QMainWindow
{
    Q_OBJECT

public:
    explicit MagicKonfug(QWidget *parent = 0);
    ~MagicKonfug();

private:
    Ui::MagicKonfug *ui;
    ConfigFileEditor configFile;
    ExeLauncher exeFile;
    static const int pageGroupCount = 5;
    static const int configEntryCount = 6;
    bool configPageMoidified[pageGroupCount];
    bool configMoidified[configEntryCount];
    bool waitingExec;

    void loadConfig();
    bool applyConfig(int configIndex);
    void setConfigModified(int configIndex, bool modified = true);
    void setConfigPageModified(int pageIndex, bool modified = true);
    void showStatusPage(bool pageVisible, QString text = QString());
    static bool testConfigFileError(ConfigFileEditor::FileErrorCode errCode,
                                    const QString& fileName,
                                    const bool aborted = true);
    static void warnMissingFile(QString fileName, bool aborted = false);
    static void warnPermission(QString objectName);
    static void warnExecPermission(QString objectName);
    static int composeKeyStringToIndex(const QString& str);
    static QString composeKeyIndexToString(int index);

private slots:
    void onExeFinished(ExeLauncher::ExecErrorCode errCode);
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
};

#endif // MagicKonfug_H
