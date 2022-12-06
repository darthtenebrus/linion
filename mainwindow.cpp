#include <QtWidgets/QMessageBox>
#include <QHash>
#include <QDesktopServices>
#include "mainwindow.h"
#include "QvObjectDelegate.h"
#include "configdialog.h"

#ifdef _DEBUG

#include <QDebug>
#include <QSettings>
#include <QDesktopServices>

#endif

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow()), configDialog(new ConfigDialog(this)),
        settings(QSettings::NativeFormat, QSettings::UserScope, "linion", "config") {
    ui->setupUi(this);

    proxyModel = new QSortFilterProxyModel(ui->addonTreeView);
    model = new QAddonListModel(fillDataFromSettings(),
                                ui->addonTreeView);

    model->setHeaderTitle(tr("Installed Addons List"));
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRole(Qt::DisplayRole);

    ui->addonTreeView->setMouseTracking(true);
    ui->addonTreeView->setModel(proxyModel);
    contextMenu = new QMenu(ui->addonTreeView);
    ui->addonTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    backupAction = new QAction(QIcon::fromTheme("folder"), tr("Backup"), contextMenu);
    reinstallAction = new QAction(QIcon::fromTheme("folder-sync"), tr("Reinstall Or Update"), contextMenu);
    uninstallAction = new QAction(QIcon::fromTheme("delete"), tr("Uninstall"), contextMenu);
    visitSiteAction = new QAction(QIcon::fromTheme("gnumeric-link-url"), tr("Visit Web site"), contextMenu);
    // folder-sync
    ui->addonTreeView->addAction(backupAction);
    ui->addonTreeView->addAction(reinstallAction);
    ui->addonTreeView->addAction(visitSiteAction);
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

    connect(configDialog, &QDialog::accepted, this, &MainWindow::configAccepted);
    connect(backupAction, &QAction::triggered, model, &QAddonListModel::backupAddonClicked);
    connect(reinstallAction, &QAction::triggered, model, &QAddonListModel::reinstallAddonClicked);
    connect(uninstallAction, &QAction::triggered, model, &QAddonListModel::uninstallAddonClicked);


    connect(visitSiteAction, &QAction::triggered, model, [=]() {

        const QModelIndexList &selectedSet = ui->addonTreeView->selectionModel()->selectedIndexes();
        if (selectedSet.count() > 1) {
            return; // fuckup
        }

        const QModelIndex &index = selectedSet[0];
        if (index.isValid()) {
            const QString &url = index.data(QAddonListModel::FileInfoURLRole).toString();
            QDesktopServices::openUrl(QUrl(url));
        }
    });

    connect(ui->addonTreeView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::currentChanged);

    connect(model, &QAddonListModel::currentRowDetailChanged,
            this, &MainWindow::currentChanged);

    connect(ui->searchEdit, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterWildcard);
    connect(ui->actionAboutQt, &QAction::triggered, this, &MainWindow::aboutQtAction);
    connect(ui->backupButton, &QToolButton::clicked, model, &QAddonListModel::backupAllClicked);
    connect(model, &QAbstractListModel::dataChanged, this, &MainWindow::allChanged);
    connect(model, &QAddonListModel::percent, this, &MainWindow::updateProgressPercent);

    connect(ui->refreshButton, &QToolButton::clicked, this, &MainWindow::refreshListClicked);
    connect(model, &QAddonListModel::backToInstalled, this, &MainWindow::refreshListClicked);

    connect(ui->findMoreButton, &QToolButton::clicked, this, [=]() {
        ui->backupButton->setVisible(false);
        model->setHeaderTitle(tr("Addons available"));
        backupAction->setEnabled(false);
        uninstallAction->setEnabled(false);
        reinstallAction->setText(tr("Install"));
        model->disconnectWatcher();
        model->refreshFromExternal();
    });
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::settingsClicked);
    connect(this, &MainWindow::setup, this, &MainWindow::settingsClicked);
    connect(ui->setupButton, &QToolButton::clicked, this, &MainWindow::settingsClicked);

    ui->backupButton->setVisible(true);

}


MainWindow::~MainWindow() {

    delete visitSiteAction;
    delete reinstallAction;
    delete uninstallAction;
    delete backupAction;
    delete progressBar;
    delete configDialog;
    delete model;
    delete proxyModel;
    delete contextMenu;
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    model->refresh();
}

void MainWindow::writeSettings(const PreferencesType &data) {

    for (const QString &key: data.keys()) {
        settings.setValue(key, data.value(key));
    }
    settings.sync();
}

void MainWindow::currentChanged(const QModelIndex &current, const QModelIndex &prev) {

    if (current.isValid()) {
        const QString &desc = current.data(QAddonListModel::DescriptionRole).toString();
        const QString &version = current.data(QAddonListModel::VersionRole).toString();
        const QString &author = current.data(QAddonListModel::AuthorRole).toString();
        ui->descriptionView->setText(desc);
        ui->descriptionView->append("\n" + tr("Version: %1").arg(version));
        ui->descriptionView->append(tr("Author: %1").arg(author));
    } else {
        ui->descriptionView->clear();
    }
}


void MainWindow::aboutQtAction(bool param) {
    QMessageBox::aboutQt(this);
}

void MainWindow::allChanged(const QModelIndex &first, const QModelIndex &last) {

    const auto *cModel = qobject_cast<const QAddonListModel *>(first.model());

    if (cModel->getAddonList().isEmpty()) {
        QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Information"),
                                                                   tr("Unable to locate addons. Do you want "
                                                                      "to open the settings dialog and "
                                                                      "choose the path to the addons folder yourself?"));
        if (answer == QMessageBox::StandardButton::Yes) {
            emit setup();
        }

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

    if (ui->backupButton->isEnabled()) {
        ui->backupButton->setEnabled(false);
    }

    if (ui->refreshButton->isEnabled()) {
        ui->refreshButton->setEnabled(false);
    }

    if (ui->findMoreButton->isEnabled()) {
        ui->findMoreButton->setEnabled(false);
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
        ui->findMoreButton->setEnabled(true);
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
    const PreferencesType &data = configDialog->receiveData();
    writeSettings(data);
    model->setModelData(data);
    refreshListClicked(true);

}

PreferencesType MainWindow::fillDataFromSettings() const {

    PreferencesType data;
    for (const QString &key: defs.keys()) {
        data.insert(key, settings.value(key, defs[key]));
    }
    return data;
}

void MainWindow::closeEvent(QCloseEvent *event) {

    for (const QString &key: defs.keys()) {
        if (!settings.contains(key)) {
            settings.setValue(key, defs[key]);
        }
    }
    settings.sync();
    QWidget::closeEvent(event);
}

void MainWindow::refreshListClicked(bool clicked) {
    ui->backupButton->setVisible(true);
    model->setHeaderTitle(tr("Installed Addons List"));
    backupAction->setEnabled(true);
    uninstallAction->setEnabled(true);
    reinstallAction->setText(tr("Reinstall Or Update"));
    model->connectWatcher();
    model->refresh();
}


