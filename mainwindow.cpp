#include <QtWidgets/QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "mainwindow.h"
#include "QvObjectDelegate.h"
#ifdef _DEBUG
#include <QDebug>
#endif

MainWindow ::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow()) {
    ui->setupUi(this);

    addonFolderPath = "/home/esorochinskiy/Games/the-elder-scrolls-online/drive_c/users/esorochinskiy/Documents/Elder Scrolls Online/live/AddOns";
    model = new QAddonListModel(addonFolderPath);
    model->fillFolderList();
    ui->addonListView->setMouseTracking(true);
    ui->addonListView->setModel(model);
    ui->addonListView->setItemDelegate(new QvObjectDelegate(ui->addonListView));
}


MainWindow :: ~MainWindow() {
    delete model;
    delete ui;
}