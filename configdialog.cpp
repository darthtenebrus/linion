//
// Created by esorochinskiy on 25.11.22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ConfigDialog.h" resolved

#include "configdialog.h"
#include "ui_configdialog.h"
#include <QPushButton>
#include <QFileDialog>

ConfigDialog::ConfigDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::ConfigDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(close()));
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(close()));
    connect(ui->listWidget->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ConfigDialog::currentChanged);

    connect(ui->addonFolderPathButton, SIGNAL(clicked()), this, SLOT(addonPathChoose()));
    connect(ui->backupPathButton, SIGNAL(clicked()), this, SLOT(backupPathChoose()));
    setTopSelected();
}

ConfigDialog::~ConfigDialog() {
    delete ui;
}

void ConfigDialog::transferData(const QHash<QString, QVariant> &data) const {
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

