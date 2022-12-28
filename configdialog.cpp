//
// Created by esorochinskiy on 25.11.22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ConfigDialog.h" resolved

#include "configdialog.h"
#include "ui_configdialog.h"
#include "DialogItemDelegate.h"
#include "preferences.h"
#include <QPushButton>
#include <QFileDialog>


QString ConfigDialog::savedVarsSuffix = "SavedVariables";

ConfigDialog::ConfigDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::ConfigDialog) {
    ui->setupUi(this);
    ui->listWidget->setItemDelegate(new DialogItemDelegate(ui->listWidget));
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &QDialog::accept);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &QDialog::reject);
    connect(ui->listWidget->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ConfigDialog::currentChanged);

    connect(ui->addonFolderPathButton, &QToolButton::clicked, this, &ConfigDialog::addonPathChoose);
    connect(ui->backupPathButton, &QToolButton::clicked, this, &ConfigDialog::backupPathChoose);
    connect(ui->savedVarsPathButton, &QToolButton::clicked, this, &ConfigDialog::savedVarsPathChoose);

    auto *locUi = ui;
    connect(ui->doNotUse, &QRadioButton::toggled, this, [locUi]() {
        locUi->tarCommand->setEnabled(false);
        locUi->zipCommand->setEnabled(false);
    });

    connect(ui->useTar, &QRadioButton::toggled, this, [locUi](bool toggled) {
        locUi->tarCommand->setEnabled(toggled);
        locUi->zipCommand->setEnabled(!toggled);
    });

    connect(ui->useZip, &QRadioButton::toggled, this, [locUi](bool toggled) {
        locUi->tarCommand->setEnabled(!toggled);
        locUi->zipCommand->setEnabled(toggled);
    });

}

ConfigDialog::~ConfigDialog() {
    delete ui;
}

void ConfigDialog::transferData(const PreferencesType &data) const {
    ui->addonFolderPath->setText(data.value("addonFolderPath").toString());
    ui->backupPath->setText(data.value("backupPath").toString());
    bool useTar = data.value("useTar").toBool();
    bool useZip = data.value("useZip").toBool();
    if (!useTar && !useZip) {
        ui->doNotUse->setChecked(true);
    }
    ui->useTar->setChecked(data.value("useTar").toBool());
    ui->useZip->setChecked(data.value("useZip").toBool());

    ui->tarCommand->setText(data.value("tarCommand").toString());
    ui->zipCommand->setText(data.value("zipCommand").toString());

    ui->tarExtractCommand->setText(data.value("tarExtractCommand").toString());
    ui->zipExtractCommand->setText(data.value("zipExtractCommand").toString());
    QString sPath = data.value("savedVarsPath").toString();
    if (sPath.isEmpty() && !ui->addonFolderPath->text().isEmpty()) {
        sPath = prepareSavedPath(ui->addonFolderPath->text());
    }
    ui->savedVarsPath->setText(sPath);
    ui->backupSaved->setChecked(data.value("backupSaved").toBool());
    ui->savedVarsPath->setEnabled(ui->backupSaved->isChecked());

}

void ConfigDialog::currentChanged(const QModelIndex &current, const QModelIndex &prev) {
    ui->stackedWidget->setCurrentIndex(current.row());
}

void ConfigDialog::setTopSelected() {
    const QModelIndex &topIndex = ui->listWidget->model()->index(0, 0);
    ui->listWidget->setCurrentIndex(topIndex);
    ui->stackedWidget->setCurrentIndex(0);
}

void ConfigDialog::addonPathChoose() {
    auto dir = QFileDialog::getExistingDirectory(this, tr("Choose ESO Adddons Directory"),
                                                 ui->addonFolderPath->text(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->addonFolderPath->setText(dir);
        const QString &sPath = prepareSavedPath(ui->addonFolderPath->text());
        ui->savedVarsPath->setText(sPath);
    }
}

void ConfigDialog::backupPathChoose() {
    auto dir = QFileDialog::getExistingDirectory(this, tr("Choose Backup Directory"),
                                                 ui->backupPath->text(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->backupPath->setText(dir);
    }
}

PreferencesType ConfigDialog::receiveData() const {

    PreferencesType data;

    data.insert("addonFolderPath", ui->addonFolderPath->text());
    data.insert("backupPath", ui->backupPath->text());
    data.insert("useTar", ui->useTar->isChecked());
    data.insert("useZip", ui->useZip->isChecked());
    data.insert("tarCommand", ui->tarCommand->text());
    data.insert("zipCommand", ui->zipCommand->text());
    data.insert("tarExtractCommand", ui->tarExtractCommand->text());
    data.insert("zipExtractCommand", ui->zipExtractCommand->text());
    data.insert("savedVarsPath", ui->savedVarsPath->text());
    data.insert("backupSaved", ui->backupSaved->isChecked());
    return data;
}

void ConfigDialog::savedVarsPathChoose() {
    auto dir = QFileDialog::getExistingDirectory(this, tr("Choose Saved Vars Directory"),
                                                 ui->savedVarsPath->text(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->savedVarsPath->setText(dir);
    }
}

QString ConfigDialog::prepareSavedPath(const QString &sPath) const {

    const QString &cPath = sPath.section(QDir::separator(), 0, -2) +
                           QDir::separator() + ConfigDialog::savedVarsSuffix;
    return  cPath;
    
}


