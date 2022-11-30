//
#ifndef LINION_MAINWINDOW_H
#define LINION_MAINWINDOW_H
#include <QMainWindow>
#include <QAction>
#include <QtNetwork/QNetworkAccessManager>
#include <QProgressBar>
#include <ui_mainwindow.h>
#include <QFileInfoList>
#include <QSettings>
#include "QAddonListModel.h"
#include "configdialog.h"

class MainWindow : public QMainWindow {
Q_OBJECT
protected:

    void showEvent(QShowEvent *event) override;

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QAddonListModel *model;
    ConfigDialog *configDialog;
    QProgressBar *progressBar;

    QAction *backupAction;
    QAction *uninstallAction;

    QSettings settings;
    PreferencesType defs = {
            {"addonFolderPath",
             QVariant("/home/esorochinskiy/Games/the-elder-scrolls-online/drive_c/users/esorochinskiy/Documents/Elder Scrolls Online/live/AddOns")},
            {"backupPath",
            QVariant("/home/esorochinskiy/ESObackup")},
            {"useTar", QVariant(true)},
            {"useZip", QVariant(false)},
            {"tarCommand", QVariant("tar cvzf %1.tgz")},
            {"zipCommand", QVariant("zip -r %1.zip")}


    };

    void writeSettings(const PreferencesType &data);

    [[nodiscard]]
    PreferencesType fillDataFromSettings() const;

signals:
    void doRefresh();
    void setup(bool clicked = true);



public slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &prev);
    void aboutQtAction(bool);
    void allChanged(const QModelIndex &first, const QModelIndex &last);
    void updateProgressPercent(int current, int total, const QString &msg);
    void settingsClicked(bool checked);
    void configAccepted();

};

#endif //TENANTCONTROL_MAINWINDOW_H