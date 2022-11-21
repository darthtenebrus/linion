//
#ifndef LINION_MAINWINDOW_H
#define LINION_MAINWINDOW_H
#include <QMainWindow>
#include <QAction>
#include <QtNetwork/QNetworkAccessManager>
#include <QProgressBar>
#include <ui_mainwindow.h>
#include <QFileInfoList>
#include "QAddonListModel.h"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QAddonListModel *model;

    QString addonFolderPath;

signals:
    void doRefresh();
};

#endif //TENANTCONTROL_MAINWINDOW_H