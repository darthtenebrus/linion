#include <QtWidgets/QMessageBox>
#include <QHash>
#include <QDesktopServices>
#include "mainwindow.h"
#include "QvObjectDelegate.h"
#include "configdialog.h"
#include "aboutdialog.h"

#ifdef _DEBUG

#include <QDebug>
#include <QSettings>
#include <QDesktopServices>

#endif

QString MainWindow::orgName = "linion";

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow()), configDialog(new ConfigDialog(this)),
        settings(QSettings::NativeFormat, QSettings::UserScope, MainWindow::orgName, "config") {
    ui->setupUi(this);

    proxyModel = new QAddonProxyModel(ui->addonTreeView);
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

    auto *delegate = new QvObjectDelegate(ui->addonTreeView);
    ui->addonTreeView->setItemDelegate(delegate);
    connect(delegate, &QvObjectDelegate::reinstallButtonClicked, model,
            &QAddonListModel::reinstallAddonClicked);
    
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

    connect(ui->searchEdit, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterWildcard);
    connect(ui->actionAboutQt, &QAction::triggered, this, &MainWindow::aboutQtAction);
    connect(ui->actionAboutApp, &QAction::triggered, this, &MainWindow::aboutAppAction);
    connect(ui->backupButton, &QToolButton::clicked, model, &QAddonListModel::backupAllClicked);
    connect(model, &QAddonListModel::addonsListChanged, this, &MainWindow::allChanged);
    connect(model, &QAddonListModel::percent, this, &MainWindow::updateProgressPercent);

    connect(ui->refreshButton, &QToolButton::clicked, this, [=]() {
        refreshListClicked();
        model->setTopIndex();
    });
    connect(model, &QAddonListModel::backToInstalled, this, &MainWindow::refreshListClicked);

    connect(ui->findMoreButton, &QToolButton::clicked, this, [=]() {
        ui->backupButton->setVisible(false);
        model->setHeaderTitle(tr("Addons available"));
        backupAction->setEnabled(false);
        uninstallAction->setEnabled(false);
        reinstallAction->setText(tr("Install"));
        model->disconnectWatcher();
        model->refreshFromExternal();
        model->setTopIndex();
    });
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::settingsClicked);
    connect(ui->setupButton, &QToolButton::clicked, this, &MainWindow::settingsClicked);

    ui->backupButton->setVisible(true);
    model->refresh();
    model->setTopIndex();
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

void MainWindow::writeSettings(const PreferencesType &data) {

    for (const QString &key: data.keys()) {
        settings.setValue(key, data.value(key));
    }
    settings.sync();
}

void MainWindow::currentChanged(const QModelIndex &current, const QModelIndex &prev) {

    ui->descriptionView->clear();
    if (current.isValid()) {
        const QString &infoUrl = current.data(QAddonListModel::FileInfoURLRole).toString();

        const QString &desc = current.data(QAddonListModel::DescriptionRole).toString();
        const QString &version = current.data(QAddonListModel::VersionRole).toString();
        const QString &author = current.data(QAddonListModel::AuthorRole).toString();
        const QString &UID = current.data(QAddonListModel::UIDRole).toString();
        ItemData::ItemStatus cStatus = current.data(QAddonListModel::StatusRole).value<ItemData::ItemStatus>();

        const QString &remoteDesc = model->tryToGetExtraData(UID, "application/json");


        const QString &urlToOpen = QString("<p><a href=\"%1\">%1</a></p>")
                .arg(infoUrl);

        ui->descriptionView->setText(urlToOpen);

        if (cStatus == ItemData::NotInstalled) {
            ui->descriptionView->append(QString("<pre>%1</pre>").arg(remoteDesc));
        } else {
            ui->descriptionView->append(QString("<b><i><p>%1</p></i></b><pre>%2</pre><b><i><p>%3</p></i></b>"
                                                "<pre>%4</pre>")
                .arg(tr("Local Description"), desc, tr("Remote Description"), remoteDesc));
        }

        ui->descriptionView->append(QString("<ul><li>%1<li>%2</ul>")
                    .arg(tr("Version: %1").arg(version),
                         tr("Author: %1").arg(author)));
    }
}


void MainWindow::aboutQtAction(bool param) {
    QMessageBox::aboutQt(this);
}

void MainWindow::allChanged() {

    if (model->getAddonList().isEmpty()) {

        ui->backupButton->setEnabled(false);
        ui->findMoreButton->setEnabled(false);
        ui->searchEdit->setEnabled(false);

        QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Information"),
                                                                   tr("Unable to locate addons. Do you want "
                                                                      "to open the settings dialog and "
                                                                      "choose the path to the addons folder yourself?"));
        if (answer == QMessageBox::StandardButton::Yes) {
            settingsClicked(true);
        }
    } else {
        ui->backupButton->setEnabled(true);
        ui->findMoreButton->setEnabled(true);
        ui->searchEdit->setEnabled(true);
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
    int res = configDialog->exec();
    if (res == QDialog::Accepted) {
        const PreferencesType &receiveData = configDialog->receiveData();
        writeSettings(receiveData);
        model->setModelData(receiveData);
        refreshListClicked();
        model->setTopIndex();
    }
    

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

void MainWindow::refreshListClicked(const QString &path) {
    ui->backupButton->setVisible(true);
    model->setHeaderTitle(tr("Installed Addons List"));
    backupAction->setEnabled(true);
    uninstallAction->setEnabled(true);
    reinstallAction->setText(tr("Reinstall Or Update"));
    model->connectWatcher();
    model->refresh();

    tryToPisitionOnInstalled(path);
}

void MainWindow::tryToPisitionOnInstalled(const QString &path) const {
    if (!path.isEmpty()) {
        const QList<ItemData> &addonList = model->getAddonList();

        int i = 0;
        bool found = false;
        for (const ItemData &o : addonList) {
            if (o.getAddonPath() == path) {
                found = true;
                break;
            }
            i++;
        }
        if (found) {
            const QModelIndex &sourceIndex = model->index(i, 0);
            if (!sourceIndex.isValid()) {
                model->setTopIndex();
                return;
            }
            const QModelIndex &destIndex = proxyModel->mapFromSource(sourceIndex);
            if (!destIndex.isValid()) {
                model->setTopIndex();
                return;
            }
            ui->addonTreeView->setCurrentIndex(destIndex);
        }

    }
}

void MainWindow::aboutAppAction(bool clicked) {
    auto *aboutApp = new AboutDialog(this);
    aboutApp->exec();
    delete aboutApp;
}


