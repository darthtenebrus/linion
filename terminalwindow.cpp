//
// Created by esorochinskiy on 20.12.22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TerminalWindow.h" resolved

#include "terminalwindow.h"
#include "ui_terminalwindow.h"


TerminalWindow::TerminalWindow(QWidget *parent) :
        QDialog(parent), ui(new Ui::TerminalWindow) {
    ui->setupUi(this);
}

TerminalWindow::~TerminalWindow() {
    delete ui;
}

void TerminalWindow::setWindowData(const QString &data, const QString &err) const {
    ui->terminalResult->setText(data);
    ui->terminalResult->append("\n");
    ui->terminalResult->append(err);
}
