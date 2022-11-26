#include <QtWidgets/QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "mainwindow.h"
#include "QvObjectDelegate.h"
#include "configdialog.h"
#ifdef _DEBUG
#include <QDebug>
#include <QSettings>

#endif

MainWindow ::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow()),configDialog(new ConfigDialog(this)),
        settings(QSettings::NativeFormat, QSettings::UserScope, "linion", "config") {
    ui->setupUi(this);

    const QString &defValue = "/home/esorochinskiy/Games/the-elder-scrolls-online/drive_c/users/esorochinskiy/Documents/Elder Scrolls Online/live/AddOns";
    const QString &defBackup = "/home/esorochinskiy/ESObackup";

    addonFolderPath = settings.value("addonFolderPath", defValue).toString();
    backupPath = settings.value("backupPath", defBackup).toString();

    useTar = settings.value("useTar", "true").toBool();
    useZip = settings.value("useZip", "false").toBool();

    tarCommand = settings.value("tarCommand", "tar cvzf %1.tgz").toString();
    zipCommand = settings.value("zipCommand", "zip -r %1.zip").toString();

#ifdef _DEBUG
    qDebug() << tarCommand;
    qDebug() << zipCommand;

#endif

    auto *model = new QAddonListModel(addonFolderPath, backupPath, tarCommand, zipCommand, useTar, useZip,
                                      ui->addonTreeView);
    
    ui->addonTreeView->setMouseTracking(true);
    ui->addonTreeView->setModel(model);
    auto contextMenu = new QMenu(ui->addonTreeView);
    ui->addonTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    backupAction = new QAction(tr("Backup"), contextMenu);
    uninstallAction = new QAction(tr("Uninstall"), contextMenu);
    ui->addonTreeView->addAction(backupAction);
    ui->addonTreeView->addAction(uninstallAction);

    progressBar = new QProgressBar(ui->statusbar);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(0);
    progressBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    progressBar->setFormat(tr("Processing: %p%"));
    progressBar->setTextVisible(true);
    progressBar->setVisible(false);
// position of progress bar should be extreme right
    ui->statusbar->addPermanentWidget(progressBar);

    ui->addonTreeView->setItemDelegate(new QvObjectDelegate(ui->addonTreeView));
    connect(this, SIGNAL(doRefresh()), model, SLOT(refresh()));
    connect(backupAction, SIGNAL(triggered()), model, SLOT(backupAddonClicked()));
    connect(uninstallAction, SIGNAL(triggered()), model, SLOT(uninstallAddonClicked()));
    connect(ui->addonTreeView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::currentChanged);

    connect(ui->actionAboutQt, SIGNAL(triggered(bool)), this, SLOT(aboutQtAction(bool)));
    connect(ui->backupButton, SIGNAL(clicked()), model, SLOT(backupAllClicked()));
    connect(model, &QAbstractListModel::dataChanged, this, &MainWindow::allChanged);
    connect(model, &QAddonListModel::percent, this, &MainWindow::updateProgressPercent);
    connect(ui->refreshButton, SIGNAL(clicked()), model, SLOT(refresh()));
    connect(ui->actionSettings, &QAction::triggered, this, [=](bool checked) {
        configDialog->show();
        configDialog->setModal(true);
    });
}


MainWindow :: ~MainWindow() {
    delete uninstallAction;
    delete backupAction;
    delete progressBar;
    delete configDialog;
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    emit doRefresh();
}

void MainWindow::closeEvent(QCloseEvent *event) {

    writeSettings();
    QWidget::closeEvent(event);
}

void MainWindow::writeSettings() {

        settings.setValue("addonFolderPath", addonFolderPath);
        settings.setValue("backupPath", backupPath);
        settings.setValue("tarCommand", tarCommand);
        settings.setValue("zipCommand", zipCommand);
        settings.setValue("useTar", useTar);
        settings.setValue("useZip", useZip);
        settings.sync();
}

void MainWindow::currentChanged(const QModelIndex &current, const QModelIndex &prev) {

    const QString &text = current.data(QAddonListModel::DescriptionRole).toString();
    ui->descriptionView->setText(text);
}


void MainWindow::aboutQtAction(bool param) {
    QMessageBox::aboutQt(this);
}

void MainWindow::allChanged(const QModelIndex &first, const QModelIndex &last) {

    const QAddonListModel *model = qobject_cast<const QAddonListModel *>(first.model());
    
    if (model->getAddonList().isEmpty()) {
        QMessageBox::critical(this, tr("Critical Error"), tr("Unable to locate addons"));
    }
}

void MainWindow::updateProgressPercent(int current, int total, const QString &msg) {
    if (!progressBar->isVisible()) {
        progressBar->setVisible(true);
    }

    if (!ui->statusbar->currentMessage().isEmpty()) {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage(msg);
    }

    if(ui->backupButton->isEnabled()) {
        ui->backupButton->setEnabled(false);
    }

    if (ui->refreshButton->isEnabled()) {
        ui->refreshButton->setEnabled(false);
    }

    int percent = qRound((float) current / (float) total * 100);

    progressBar->setValue(percent);

    if (percent == 100) {
        progressBar->setVisible(false);
        ui->statusbar->showMessage(tr("All completed"));
        ui->backupButton->setEnabled(true);
        ui->refreshButton->setEnabled(true);
    }
}
