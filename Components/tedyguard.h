#ifndef TEDYGUARD_H
#define TEDYGUARD_H

#include <QMainWindow>
#include "filecleaner.h"


namespace Ui {
class TedyGuard;
}

class TedyGuard : public QMainWindow
{
    Q_OBJECT

public:
    explicit TedyGuard(QWidget *parent = 0);
    ~TedyGuard();

private:
    Ui::TedyGuard *ui;
    FileCleaner cleaner;
    int currentState;

private slots:
    void onCleanerScanFinished();
    void onCleanerCleaningFinished();

    void on_buttonScan_clicked();
    void on_buttonStopScan_clicked();
    void on_buttonCancelClean_clicked();
    void on_buttonClean_clicked();
    void on_buttonStopClean_clicked();
    void on_buttonReturn_clicked();
};

#endif // TEDYGUARD_H
