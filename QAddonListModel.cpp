//
// Created by esorochinskiy on 20.11.22.
//

#include "QAddonListModel.h"
#include "preferences.h"
#include <QPixmap>

#ifdef _DEBUG

#include <QDebug>

#endif

#include <QRegularExpression>
#include <QListView>
#include <QtWidgets/QMessageBox>
#include <QFileSystemWatcher>
#include <QTreeView>
#include <QProcess>

QString QAddonListModel::listUrl = "https://api.mmoui.com/v3/game/ESO/filelist.json";

QAddonListModel::QAddonListModel(const PreferencesType &settings, QObject *parent)
        : QAbstractListModel(parent), manager(new QNetworkAccessManager(this)) {

    setModelData(settings);
    qsw = new QFileSystemWatcher(QStringList() << this->addonFolderPath << this->backupPath);
    connect(qsw, &QFileSystemWatcher::directoryChanged, this, &QAddonListModel::refresh);
    connect(manager, &QNetworkAccessManager::finished, this, &QAddonListModel::replyFinished);

}

QAddonListModel::~QAddonListModel() {
    qsw->removePath(addonFolderPath);
    qsw->removePath(backupPath);
    delete qsw;
    delete manager;
}

int QAddonListModel::rowCount(const QModelIndex &) const {
    return addonList.count();
}

QVariant QAddonListModel::data(const QModelIndex &index, int role) const {

    QVariant value;
    if (!index.isValid()) {
        return value;
    }

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
        case QAddonListModel::DescriptionRole:
            value = addonList.at(index.row()).getDescription();
            break;
        case Qt::DecorationRole:
            value = QPixmap(addonList.at(index.row()).isStatus() == ItemData::InstalledBackedUp ?
                            ":/images/green_check.png" : ":/images/red_cross.png");
            break;
        case QAddonListModel::DownloadTotalRole:
            value = addonList.at(index.row()).getDownloadTotal();
            break;
        case QAddonListModel::DownloadMonthlyRole:
            value = addonList.at(index.row()).getDownloadMonthly();
            break;
        case QAddonListModel::FavoriteTotalRole:
            value = addonList.at(index.row()).getFavoriteTotal();
            break;
        case QAddonListModel::FileInfoURLRole:
            value = addonList.at(index.row()).getFileInfoUrl();
            break;
        default:
            break;
    }

    return value;
}

void QAddonListModel::refreshFolderList() {
    QRegularExpression re(R"(##\s+(?<tag>[A-Za-z]+):\s+(?<content>.*))");
    int totalCount = addonList.count();
    beginRemoveRows(QModelIndex(), 0, totalCount >= 1 ? totalCount - 1 : 0);
    addonList.clear();
    endRemoveRows();
    QDir dir = QDir(addonFolderPath);
    const QFileInfoList &dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    int total = dirList.count();
    beginInsertRows(QModelIndex(), 0, dirList.count() - 1);
    for (int i = 0; i < total; i++) {
        emit percent(i, total, tr("Refresh"));
        const QString &addonName = dirList.at(i).fileName();
        auto separ = QDir::separator();
        const QString &fPath = addonFolderPath + separ + addonName + separ + addonName + ".txt";
        QFile file(fPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
#ifdef _DEBUG
            qDebug() << "Failed to open input file.";
            continue;
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
            QString description;
            while (!file.atEnd()) {
                QString line = file.readLine();
                QRegularExpressionMatch match = re.match(line);

                if (match.hasMatch()) {
                    const QString &tag = match.captured("tag");
                    const QString &content = match.captured("content");


                    if (QString::compare("Title", tag) == 0) {
                        title = content;
                    } else if (QString::compare("Version", tag) == 0) {
                        version = content;
                    } else if (QString::compare("Author", tag) == 0) {
                        author = content;
                    } else if (QString::compare("Description", tag) == 0) {
                        description = content;
                    }

                    if (!title.isEmpty() && !version.isEmpty() &&
                        !author.isEmpty() && !description.isEmpty()) {
                        break;
                    }
                }
            }

            if (!title.isEmpty() && !version.isEmpty() && !author.isEmpty()) {

                const QString &finalDesc = description.isEmpty() ? "[" + tr("No description") + "]" : description;
                ItemData::ItemStatus backupStatus = checkBackupStatus(addonName);
                auto foundNetData = std::find_if(esoSiteList.begin(), esoSiteList.end(), [&addonName](QJsonObject o) {

                    return o.value("UIDir").toArray()[0] == addonName;
                });
                if (foundNetData != esoSiteList.end()) {

                    addonList.append(ItemData(cleanColorizers(author), cleanColorizers(title),
                                              version, fPath,
                                              finalDesc,
                                              backupStatus,
                                              foundNetData->value("UIDownloadTotal").toString("0"),
                                              foundNetData->value("UIDownloadMonthly").toString("0"),
                                              foundNetData->value("UIFavoriteTotal").toString("0"),
                                              foundNetData->value("UIFileInfoURL").toString()));
                }
            }
        }

    }
    endInsertRows();
    emit dataChanged(createIndex(0, 0), createIndex(addonList.count() - 1, 0));
    emit percent(total, total, "");
}

const QString &QAddonListModel::cleanColorizers(QString &input) const {
    return input.replace(
                    QRegularExpression(R"(\|c[A-Za-z0-9]{6})"), "")
            .replace("|r", "")
            .replace(QRegularExpression(R"(\|t.*?\|t)"), "");
}

void QAddonListModel::refresh() {
    if (esoSiteList.isEmpty()) {
        refreshESOSiteList();
    } else {
        refreshFolderList();
        setTopIndex();
    }
}

void QAddonListModel::uninstallAddonClicked() {

    auto *view = qobject_cast<QTreeView *>(parent());
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

    QMessageBox::StandardButton button = QMessageBox::warning(view, tr("Info"),
                                                              tr("Do you really want to delete this addon: %1?")
                                                                      .arg(aTitle),
                                                              QMessageBox::StandardButtons(
                                                                      QMessageBox::Yes | QMessageBox::No));
    if (button == QMessageBox::Yes) {
        const QString &parPath = QFileInfo(aPath).absolutePath();
        QDir(parPath).removeRecursively();

#ifdef _DEBUG
        qDebug() << parPath;
#endif
    }

}

void QAddonListModel::backupAllClicked() {

    auto *view = qobject_cast<QTreeView *>(parent());
    QMessageBox::StandardButton button = QMessageBox::warning(view, tr("Info"),
                                                              tr("Do you want to make a backup of all installed addons?"),
                                                              QMessageBox::StandardButtons(
                                                                      QMessageBox::Yes | QMessageBox::No));
    if (button == QMessageBox::No) {
        return;
    }

    int total = addonList.size();
    for (int i = 0; i < total; i++) {
        emit percent(i, total, tr("Backing up"));
        processBackup(addonList[i].getAddonPath());
    }
    emit percent(total, total, "");
}

void QAddonListModel::backupAddonClicked() {

    const QTreeView *view = qobject_cast<QTreeView *>(parent());
    const QModelIndexList &selectedSet = view->selectionModel()->selectedIndexes();
    if (selectedSet.count() > 1) {
        return; // fuckup
    }
    const QModelIndex &index = selectedSet[0];
    const QString &aPath = index.data(QAddonListModel::PathRole).toString();

    emit percent(1, 100, tr("Backing up single addon"));
    processBackup(aPath);
    emit percent(100, 100, tr("Backing up single addon"));
}

void QAddonListModel::processBackup(const QString &pPath) const {

    const QString &parPath = QFileInfo(pPath).absolutePath();
    const QDir &srcDir = QDir(parPath);
    const QDir &destDir = QDir(backupPath + QDir::separator() + srcDir.dirName());

#ifdef _DEBUG
    qDebug() << useTar;
    qDebug() << useZip;
#endif

    if (!destDir.exists()) {
        destDir.mkpath(".");
    }
    copyPath(srcDir.absolutePath(), destDir.absolutePath());

    if (useZip || useTar) {
        QProcess proc;
        proc.setWorkingDirectory(backupPath);
        if (useTar) {
            // имя архива. Пробелы убираем
            const QString &newTarCommand = QString(tarCommand)
                    .arg(destDir.dirName().replace(QRegularExpression(R"(\s+)"), ""));
            QStringList commandList = newTarCommand.trimmed().split(QRegularExpression(R"(\s+)"));
#ifdef _DEBUG
            qDebug() << commandList;
#endif
            const QString &command = commandList.value(0);
            commandList.removeAt(0);
            const QString &dDir = destDir.dirName();
            commandList << (!dDir.contains(" ") ? dDir : R"(")" + dDir + R"(")");
#ifdef _DEBUG
            qDebug() << "command = " + command;
            qDebug() << commandList;

#endif
            proc.start(command, commandList);
            proc.waitForFinished();
            QProcess::ProcessError res = proc.error();
            const QString &strRes = QString(proc.readAllStandardOutput());
            const QString &errRes = QString(proc.readAllStandardError());
#ifdef _DEBUG
            qDebug() << res;
            qDebug() << strRes;
            qDebug() << errRes;
#endif
            QDir(destDir).removeRecursively();

        } else if (useZip) {
            // имя архива. Пробелы убираем
            const QString &newZipCommand = QString(zipCommand)
                    .arg(destDir.dirName().replace(QRegularExpression(R"(\s+)"), ""));
            QStringList commandList = newZipCommand.trimmed().split(QRegularExpression(R"(\s+)"));
#ifdef _DEBUG
            qDebug() << commandList;
#endif
            const QString &command = commandList.value(0);
            commandList.removeAt(0);
            const QString &dDir = destDir.dirName();
            commandList << (!dDir.contains(" ") ? dDir : R"(")" + dDir + R"(")");
#ifdef _DEBUG
            qDebug() << "command = " + command;
            qDebug() << commandList;

#endif
            proc.start(command, commandList);
            proc.waitForFinished();
            QProcess::ProcessError res = proc.error();
            const QString &strRes = QString(proc.readAllStandardOutput());
            const QString &errRes = QString(proc.readAllStandardError());
#ifdef _DEBUG
            qDebug() << res;
            qDebug() << strRes;
            qDebug() << errRes;
#endif
            QDir(destDir).removeRecursively();


        }
    }
}


void QAddonListModel::copyPath(const QString &src, const QString &dst) const {

    QDir srcDir(src);
    if (!srcDir.exists()) {
        return;
    }

    for (const QString &f: srcDir.entryList(QDir::Files)) {
        QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }

    for (const QString &d: srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString &new_src_path = src + QDir::separator() + d;
        const QString &new_dst_path = dst + QDir::separator() + d;
        QDir(new_dst_path).mkpath(".");
        copyPath(new_src_path, new_dst_path);
    }
}

const QList<ItemData> &QAddonListModel::getAddonList() const {
    return addonList;
}

QVariant QAddonListModel::headerData(int section, Qt::Orientation orientation, int role) const {

    QVariant value;
    switch (role) {
        case Qt::DisplayRole:
            value = tr("Installed Addons List");
    }
    return value;
}

void QAddonListModel::sort(int column, Qt::SortOrder order) {

    emit layoutAboutToBeChanged();
    qSort(addonList.begin(), addonList.end(), [&order](ItemData &v1, ItemData &v2) {
        return order == Qt::AscendingOrder ? v1.getAddonTitle() < v2.getAddonTitle() : v1.getAddonTitle() >
                                                                                       v2.getAddonTitle();
    });
    emit layoutChanged();

}

void QAddonListModel::setModelData(const PreferencesType &data) {

    addonFolderPath = data.value("addonFolderPath").toString();
    backupPath = data.value("backupPath").toString();

    useTar = data.value("useTar").toBool();
    useZip = data.value("useZip").toBool();

    tarCommand = data.value("tarCommand").toString();
    zipCommand = data.value("zipCommand").toString();

}

ItemData::ItemStatus QAddonListModel::checkBackupStatus(const QString &aName) const {

    if (useTar) {
        if (QFile(backupPath + QDir::separator() + aName + ".tgz").exists()) {
            return ItemData::InstalledBackedUp;
        } else {
            return ItemData::Installed;
        }
    } else if (useZip) {
        if (QFile(backupPath + QDir::separator() + aName + ".zip").exists()) {
            return ItemData::InstalledBackedUp;
        } else {
            return ItemData::Installed;
        }
    } else {
        if (QDir(backupPath + QDir::separator() + aName).exists()) {
            return ItemData::InstalledBackedUp;
        } else {
            return ItemData::Installed;
        }
    }

}

void QAddonListModel::replyFinished(QNetworkReply *reply) {

    emit percent(100, 100, "");
    QNetworkReply::NetworkError error = reply->error();
    if (error == QNetworkReply::NetworkError::NoError) {

        const QString &senderUrl = reply->request().url().toString();

        if (listUrl == senderUrl) {
            const QByteArray &byteres = reply->readAll();
            const QJsonDocument &document = QJsonDocument::fromJson(byteres);
            if (!document.isEmpty() && document.isArray()) {
                const QJsonArray &dataArray = document.array();
                for (QJsonValue v: dataArray)
                    if (v.isObject()) {
                        esoSiteList.append(v.toObject());
                    }

                refreshFolderList();
                setTopIndex();
            } else {
                QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                                      tr("Invalid data"));
            }
        }
    } else {
        QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                              reply->errorString());
    }

}

void QAddonListModel::refreshESOSiteList() {
    emit percent(0, 100, tr("Updating data"));
    QNetworkRequest request = QNetworkRequest(QUrl(listUrl));
    request.setRawHeader("Content-Type", "application/json");
    manager->get(QNetworkRequest(request));
}

void QAddonListModel::setTopIndex() {
    const QModelIndex &index = this->index(0, 0);
    if (index.isValid()) {
        auto *view = qobject_cast<QTreeView *>(parent());
        view->setCurrentIndex(index);
    }
}

void QAddonListModel::reinstallAddonClicked() {
    const QTreeView *view = qobject_cast<QTreeView *>(parent());
    const QModelIndexList &selectedSet = view->selectionModel()->selectedIndexes();
    if (selectedSet.count() > 1) {
        return; // fuckup
    }

    const QModelIndex &index = selectedSet[0];
    if (index.isValid()) {
        QRegExp rx(R"(info([0-9]*)-([^\.]+)\.html)");
        QString &&urlPath = index.data(QAddonListModel::FileInfoURLRole).toString();
        QString &downPath = urlPath.replace(rx, R"(dl\1/\2.zip)");

        auto *tmpRedirectManager = new QNetworkAccessManager(this);
        connect(tmpRedirectManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {

            emit percent(100, 100, "");
            QNetworkReply::NetworkError error = reply->error();

            if (error == QNetworkReply::NetworkError::NoError) {

                const QString &aPath = index.data(QAddonListModel::PathRole).toString();
                const QString &pPath = QFileInfo(aPath).absolutePath() + ".zip";

                /*
int httpStatus = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
const QString &httpStatusMessage = reply->attribute(
        QNetworkRequest::HttpReasonPhraseAttribute).toString();
const QVariant &loc = reply->header(QNetworkRequest::LocationHeader);

#ifdef _DEBUG
qDebug() << httpStatus << httpStatusMessage << loc;
#endif

 */

                QFile file(pPath);

                if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
                    QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                                          tr("I/O error"));
                    return;
                }

                const QByteArray &arr = reply->readAll();

                file.write(arr);
                file.flush();
                file.close();
            } else {
                QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                                      reply->errorString());
            }
        });
        emit percent(0, 100, tr("Downloading"));
        QNetworkRequest &&request = QNetworkRequest(QUrl(downPath));
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        tmpRedirectManager->get(request);

    }
}


