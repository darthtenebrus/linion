//
// Created by esorochinskiy on 13.12.22.
//

#ifndef LINION_ABOUTDIALOG_H
#define LINION_ABOUTDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class AboutDialog; }
QT_END_NAMESPACE

class AboutDialog : public QDialog {
Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

    ~AboutDialog() override;

private:
    Ui::AboutDialog *ui;
};


#endif //LINION_ABOUTDIALOG_H
