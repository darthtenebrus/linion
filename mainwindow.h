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
#include <QSortFilterProxyModel>
#include "QAddonListModel.h"
#include "configdialog.h"
#include "QAddonProxyModel.h"

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    QAddonListModel *model;
    QAddonProxyModel *proxyModel;
    ConfigDialog *configDialog;
    QProgressBar *progressBar;
    QMenu *contextMenu;

    QAction *backupAction;
    QAction *uninstallAction;
    QAction *reinstallAction;
    QAction *visitSiteAction;

    QSettings settings;
    PreferencesType defs = {
            {"addonFolderPath",
             QVariant(QDir::homePath())},
            {"backupPath",
            QVariant(QDir::homePath() + QDir::separator() + "ESObackup")},
            {"useTar", QVariant(true)},
            {"useZip", QVariant(false)},
            {"tarCommand", QVariant("tar cvzf %1.tgz")},
            {"zipCommand", QVariant("zip -r %1.zip")},
            {"zipExtractCommand", QVariant("unzip -o")}


    };

    void writeSettings(const PreferencesType &data);

    [[nodiscard]]
    PreferencesType fillDataFromSettings() const;

signals:
    void setup(bool clicked = true);



public slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &prev);
    void aboutQtAction(bool);
    void allChanged();
    void updateProgressPercent(int current, int total, const QString &msg);
    void settingsClicked(bool checked);
    void refreshListClicked(bool);
    void configAccepted();

};

#endif //TENANTCONTROL_MAINWINDOW_H