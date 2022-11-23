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
    const QString &defBackup = "/home/esorochinskiy/ESObackup";

    addonFolderPath = settings.value("addonFolderPath", defValue).toString();
    backupPath = settings.value("backupPath", defBackup).toString();

    model = new QAddonListModel(addonFolderPath, backupPath, ui->addonListView);

    ui->addonListView->setMouseTracking(true);
    ui->addonListView->setModel(model);
    auto contextMenu = new QMenu(ui->addonListView);
    ui->addonListView->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto *backupAction = new QAction(tr("Backup"), contextMenu);
    auto *uninstallAction = new QAction(tr("Uninstall"), contextMenu);
    ui->addonListView->addAction(backupAction);
    ui->addonListView->addAction(uninstallAction);
    ui->addonListView->setItemDelegate(new QvObjectDelegate(ui->addonListView));
    connect(this, SIGNAL(doRefresh()), model, SLOT(refresh()));
    connect(backupAction, SIGNAL(triggered()), model, SLOT(backupAddonClicked()));
    connect(uninstallAction, SIGNAL(triggered()), model, SLOT(uninstallAddonClicked()));
    connect(ui->addonListView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged(QModelIndex,QModelIndex)));

    connect(ui->actionAboutQt, SIGNAL(triggered(bool)), this, SLOT(aboutQtAction(bool)));


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

    bool needSync = false;
    if (!settings.contains("addonFolderPath")) {
        settings.setValue("addonFolderPath", addonFolderPath);
        needSync = true;
    }

    if (!settings.contains("backupPath")) {
        settings.setValue("backupPath", backupPath);
        needSync = true;
    }

    if (needSync) {
        settings.sync();
    }
}

void MainWindow::currentChanged(QModelIndex current, QModelIndex prev) {

    const QString &text = current.data(QAddonListModel::DescriptionRole).toString();
    ui->descriptionView->setText(text);
}


void MainWindow::aboutQtAction(bool param) {
    QMessageBox::aboutQt(this);
}
