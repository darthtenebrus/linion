#include <QtWidgets/QMessageBox>
#include <QHash>
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

    model = new QAddonListModel(fillDataFromSettings(),
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
    connect(configDialog, &QDialog::accepted, this, &MainWindow::configAccepted);
    connect(backupAction, SIGNAL(triggered()), model, SLOT(backupAddonClicked()));
    connect(uninstallAction, SIGNAL(triggered()), model, SLOT(uninstallAddonClicked()));
    connect(ui->addonTreeView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::currentChanged);

    connect(ui->actionAboutQt, SIGNAL(triggered(bool)), this, SLOT(aboutQtAction(bool)));
    connect(ui->backupButton, SIGNAL(clicked()), model, SLOT(backupAllClicked()));
    connect(model, &QAbstractListModel::dataChanged, this, &MainWindow::allChanged);
    connect(model, &QAddonListModel::percent, this, &MainWindow::updateProgressPercent);
    connect(ui->refreshButton, SIGNAL(clicked()), model, SLOT(refresh()));
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::settingsClicked);
    connect(ui->setupButton, &QToolButton::clicked, this, &MainWindow::settingsClicked);
}


MainWindow :: ~MainWindow() {
    delete uninstallAction;
    delete backupAction;
    delete progressBar;
    delete configDialog;
    delete model;
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    emit doRefresh();
}

void MainWindow::writeSettings(const PreferencesType &data) {

    for(const QString& key : data.keys()) {
            settings.setValue(key, data.value(key));
        }
        settings.sync();
}

void MainWindow::currentChanged(const QModelIndex &current, const QModelIndex &prev) {

    const QString &desc = current.data(QAddonListModel::DescriptionRole).toString();
    const QString &version = current.data(QAddonListModel::VersionRole).toString();
    const QString &author = current.data(QAddonListModel::AuthorRole).toString();
    ui->descriptionView->setText(desc);
    ui->descriptionView->append("\n" + tr("Version: %1").arg(version));
    ui->descriptionView->append(tr("Author: %1").arg(author));

}


void MainWindow::aboutQtAction(bool param) {
    QMessageBox::aboutQt(this);
}

void MainWindow::allChanged(const QModelIndex &first, const QModelIndex &last) {

    const auto *model = qobject_cast<const QAddonListModel *>(first.model());
    
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

    if (ui->setupButton->isEnabled()) {
        ui->setupButton->setEnabled(false);
    }

    int percent = qRound((float) current / (float) total * 100);

    progressBar->setValue(percent);

    if (percent == 100) {
        progressBar->setVisible(false);
        ui->statusbar->showMessage(tr("All completed"));
        ui->backupButton->setEnabled(true);
        ui->refreshButton->setEnabled(true);
        ui->setupButton->setEnabled(true);
    }
}

void MainWindow::settingsClicked(bool) {
    const PreferencesType &data = fillDataFromSettings();
    configDialog->transferData(data);
    configDialog->setTopSelected();
    configDialog->setModal(true);
    configDialog->show();

}

void MainWindow::configAccepted() {
#ifdef _DEBUG
    qDebug() << "Accepted";
#endif
    const PreferencesType  &data = configDialog->receiveData();
    writeSettings(data);
    model->setModelData(data);
    emit doRefresh();

}

PreferencesType MainWindow::fillDataFromSettings() const {

    PreferencesType data;
    for(const QString& key : settings.allKeys()) {
        data.insert(key, settings.value(key, defs[key]));
    }
    return data;
}


