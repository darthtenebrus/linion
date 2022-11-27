//
// Created by esorochinskiy on 25.11.22.
//

#ifndef LINION_CONFIGDIALOG_H
#define LINION_CONFIGDIALOG_H

#include <QDialog>
#include "preferences.h"


QT_BEGIN_NAMESPACE
namespace Ui { class ConfigDialog; }
QT_END_NAMESPACE

class ConfigDialog : public QDialog {
Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog() override;
    void setTopSelected();
    void transferData(const PreferencesType &data) const;

    [[nodiscard]]
    PreferencesType receiveData() const;


private:
    Ui::ConfigDialog *ui;


private slots:
    void currentChanged(const QModelIndex &, const QModelIndex &);
    void addonPathChoose();
    void backupPathChoose();


};


#endif //LINION_CONFIGDIALOG_H
