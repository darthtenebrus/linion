//
// Created by esorochinskiy on 20.11.22.
//

#include "QAddonListModel.h"
#include <QPixmap>
#ifdef _DEBUG
#include <QDebug>
#endif
#include <QRegularExpression>
#include <QListView>
#include <QtWidgets/QMessageBox>



QAddonListModel::QAddonListModel(QString &addonFolderPath, QObject *parent) : QAbstractListModel(parent),
                                                                              addonFolderPath(addonFolderPath) {

}

QAddonListModel::~QAddonListModel() {
}

int QAddonListModel::rowCount(const QModelIndex &) const {
    return addonList.count();
}

QVariant QAddonListModel::data(const QModelIndex &index, int role) const {

    QVariant value;
    switch (role) {
        case Qt::DisplayRole: //string
            value = addonList.at(index.row()).getAddonTitle();
            break;
        case QAddonListModel::VersionRole:
            value = addonList.at(index.row()).getVersion();
            break;
        case QAddonListModel::PathRole:
            value = addonList.at(index.row()).getAddonPath();
            break;
        case QAddonListModel::AuthorRole:
            value = addonList.at(index.row()).getAuthor();
            break;
        case Qt::DecorationRole:
            value = QPixmap(addonList.at(index.row()).isStatus() ?
                    ":/images/green_check.png" : ":/images/red_cross.png");
        default:
            break;
    }

    return value;
}

const QList<ItemData> & QAddonListModel::refreshFolderList() {
    QRegularExpression re(R"(##\s+(?<tag>[A-Za-z]+):\s+(?<content>.*))");
    addonList.clear();
    QDir dir = QDir(addonFolderPath);
    const QFileInfoList &dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirList.count(); i++) {
        const QString &addonName = dirList.at(i).fileName();
        const QString &fPath = addonFolderPath + "/" + addonName + "/" + addonName + ".txt";
        QFile file(fPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
#ifdef _DEBUG
            qDebug() << "Failed to open input file.";
#endif
        } else {
            if (!re.isValid()) {
#ifdef _DEBUG
                qDebug() << re.errorString();
#endif
            }

            QString title;
            QString version;
            QString author;
            while (!file.atEnd()) {
                QString line = file.readLine();
                QRegularExpressionMatch match = re.match(line);

                if (match.hasMatch()) {
                    const QString &tag = match.captured("tag");
                    const QString &content = match.captured("content");


                    if(QString::compare("Title", tag) == 0) {
                        title = content;
                        if (!version.isEmpty() && !author.isEmpty()) {
                            break;
                        }
                    } else if (QString::compare("Version", tag) == 0) {
                        version = content;
                        if (!title.isEmpty() && !author.isEmpty()) {
                            break;
                        }
                    } else if (QString::compare("Author", tag) == 0) {
                        author = content;
                        if (!title.isEmpty() && !version.isEmpty()) {
                            break;
                        }
                    }
                }
            }
#ifdef _DEBUG
            qDebug() << title << " " << version;
#endif
            if (!title.isEmpty() && !version.isEmpty()) {
                addonList.append(ItemData(cleanColorizers(author), cleanColorizers(title),
                                          version, fPath, true));
            }
        }

    }

    emit dataChanged(createIndex(0,0), createIndex(addonList.count() - 1, 0));
    return addonList;
}

const QString &QAddonListModel::cleanColorizers(QString &input) const {
    return input.replace(
                    QRegularExpression(R"(\|c[A-Za-z0-9]{6})"), "")
            .replace("|r", "")
            .replace(QRegularExpression(R"(\|t.*?\|t)"), "");
}

void QAddonListModel::refresh() {
    refreshFolderList();
}

void QAddonListModel::uninstallAddonClicked() {

    QListView *view = static_cast<QListView *>(parent());
    const QModelIndexList &selectedSet = view->selectionModel()->selectedIndexes();

    if (selectedSet.count() > 1) {
        return; // fuckup
    }

    const QModelIndex &index = selectedSet[0];
    const QString &aPath = index.data(QAddonListModel::PathRole).toString();
    const QString &aTitle = index.data(Qt::DisplayRole).toString();
#ifdef _DEBUG
    qDebug() << aPath;
    qDebug() << aTitle;
#endif

    QMessageBox::StandardButton button = QMessageBox::question(view, tr("Info"),
                                                               tr("Do you really want to delete this addon: %1")
                                                               .arg(aTitle));
    if (button == QMessageBox::Yes) {
        const QString &parPath = QFileInfo(aPath).absolutePath();
#ifdef _DEBUG
        qDebug() << parPath;
#endif
    }

}

