//
// Created by esorochinskiy on 25.11.22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ConfigDialog.h" resolved

#include "configdialog.h"
#include "ui_configdialog.h"
#include <QPushButton>

ConfigDialog::ConfigDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::ConfigDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(close()));
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(close()));
}

ConfigDialog::~ConfigDialog() {
    delete ui;
}
