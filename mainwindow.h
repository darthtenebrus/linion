//
#ifndef LINION_MAINWINDOW_H
#define LINION_MAINWINDOW_H
#include <QMainWindow>
#include <QAction>
#include <QtNetwork/QNetworkAccessManager>
#include <QProgressBar>
#include <ui_mainwindow.h>
#include <QFileInfoList>
#include <QSettings>
#include "QAddonListModel.h"

class MainWindow : public QMainWindow {
Q_OBJECT
protected:
    void closeEvent(QCloseEvent *event) override;

public:
    MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QProgressBar *progressBar;
    QString addonFolderPath;
    QString backupPath;
    QSettings settings;

    void writeSettings();

signals:
    void doRefresh();


public slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &prev);
    void aboutQtAction(bool);
    void allChanged(const QModelIndex &first, const QModelIndex &last);
    void updateProgressPercent(int current, int total);
};

#endif //TENANTCONTROL_MAINWINDOW_H