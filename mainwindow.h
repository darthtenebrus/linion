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
    void closeEvent(QCloseEvent *event) override;

    void showEvent(QShowEvent *event) override;

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ConfigDialog *configDialog;
    QProgressBar *progressBar;
    QString addonFolderPath;
    QString backupPath;
    QSettings settings;
    bool useTar;
    bool useZip;
    QString tarCommand;
    QString zipCommand;
    QAction *backupAction;
    QAction *uninstallAction;

    void writeSettings();

signals:
    void doRefresh();


public slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &prev);
    void aboutQtAction(bool);
    void allChanged(const QModelIndex &first, const QModelIndex &last);
    void updateProgressPercent(int current, int total, const QString &msg);



};

#endif //TENANTCONTROL_MAINWINDOW_H