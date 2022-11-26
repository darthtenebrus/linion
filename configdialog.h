//
// Created by esorochinskiy on 25.11.22.
//

#ifndef LINION_CONFIGDIALOG_H
#define LINION_CONFIGDIALOG_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class ConfigDialog; }
QT_END_NAMESPACE

class ConfigDialog : public QDialog {
Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog() override;
    void transferData(const QHash<QString, QVariant> &data) const;


private:
    Ui::ConfigDialog *ui;
    void setTopSelected();

private slots:
    void currentChanged(const QModelIndex &, const QModelIndex &);


};


#endif //LINION_CONFIGDIALOG_H
