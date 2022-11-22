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
    QString addonFolderPath;
    QSettings settings;
    QAddonListModel *model;
    void writeSettings();

signals:
    void doRefresh();



};

#endif //TENANTCONTROL_MAINWINDOW_H