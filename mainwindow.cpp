#include <QtWidgets/QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "mainwindow.h"
#include "QvObjectDelegate.h"
#ifdef _DEBUG
#include <QDebug>
#include <QSettings>

#endif

MainWindow ::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow()),
        settings(QSettings::NativeFormat, QSettings::UserScope, "linion", "config") {
    ui->setupUi(this);

    const QString &defValue = "/home/esorochinskiy/Games/the-elder-scrolls-online/drive_c/users/esorochinskiy/Documents/Elder Scrolls Online/live/AddOns";

    addonFolderPath = settings.value("addonFolderPath", defValue).toString();
    model = new QAddonListModel(addonFolderPath, ui->addonListView);

    ui->addonListView->setMouseTracking(true);
    ui->addonListView->setModel(model);
    auto contextMenu = new QMenu(ui->addonListView);
    ui->addonListView->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto *uninstallAction = new QAction(tr("Uninstall Addon"), contextMenu);
    ui->addonListView->addAction(uninstallAction);
    ui->addonListView->setItemDelegate(new QvObjectDelegate(ui->addonListView));
    connect(this, SIGNAL(doRefresh()), model, SLOT(refresh()));
    connect(uninstallAction, SIGNAL(triggered()), model, SLOT(uninstallAddonClicked()));
    emit doRefresh();
}


MainWindow :: ~MainWindow() {
    delete model;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {

    writeSettings();
    QWidget::closeEvent(event);
}

void MainWindow::writeSettings() {
    if (!settings.contains("addonFolderPath")) {
        settings.setValue("addonFolderPath", addonFolderPath);
        settings.sync();
    }
}
