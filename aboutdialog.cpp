//
// Created by esorochinskiy on 13.12.22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AboutDialog.h" resolved

#include "aboutdialog.h"
#include "ui_aboutdialog.h"


AboutDialog::AboutDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::AboutDialog) {
    ui->setupUi(this);
}

AboutDialog::~AboutDialog() {
    delete ui;
}
