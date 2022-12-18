//
// Created by esorochinskiy on 20.11.22.
//

#include "QAddonListModel.h"
#include "preferences.h"
#include "BinaryDownloader.h"
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
#include <QEventLoop>
#include <QToolButton>
#include <QCoreApplication>


QString QAddonListModel::listUrl = "https://api.mmoui.com/v3/game/ESO/filelist.json";
QString QAddonListModel::detailsUrl = "https://api.mmoui.com/v3/game/ESO/filedetails/";
QStringList QAddonListModel::restrictedCategs = QStringList() << "35" << "88";

QAddonListModel::QAddonListModel(const PreferencesType &settings, QObject *parent)
        : QAbstractListModel(parent) {

    setModelData(settings);
    connectWatcher();
    bdl = new BinaryDownloader(QAddonListModel::listUrl, "application/json", this);

}

QAddonListModel::~QAddonListModel() {
    delete bdl;
    disconnectWatcher();
}

int QAddonListModel::rowCount(const QModelIndex &) const {
    return addonList.count();
}

int QAddonListModel::columnCount(const QModelIndex &parent) const {
    return 1;
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
        case QAddonListModel::SiteVersionRole:
            value = addonList.at(index.row()).getSiteVersion();
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
        case Qt::DecorationRole: {
            const ItemData::ItemStatus &cStatus = addonList.at(index.row()).isStatus();
            if (cStatus != ItemData::NotInstalled) {
                value = QPixmap(addonList.at(index.row()).isStatus() == ItemData::InstalledBackedUp ?
                                ":/images/green_check.png" : ":/images/red_cross.png");
            } else {
                value = QPixmap(":/images/red_cross.png");
            }
        }
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
        case QAddonListModel::StatusRole:
            value = addonList.at(index.row()).isStatus();
            break;
        case QAddonListModel::UIDRole:
            value = addonList.at(index.row()).getUid();
        default:
            break;
    }

    return value;
}

ItemData *QAddonListModel::prepareAndFillDataByAddonName(const QString &addonName) const {
    QRegularExpression re(R"(##\s+(?<tag>[A-Za-z]+):\s+(?<content>.*))");

    auto separ = QDir::separator();
    const QString &fPath = addonFolderPath + separ + addonName + separ + addonName + ".txt";
    QFile file(fPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return nullptr;
    } else {

        // This fucking MAC OS doesn't allow to use text mode read
        QString title;
        QString version;
        QString author;
        QString description;
        file.setTextModeEnabled(false);
        const QString &allData = file.readAll();
        const QStringList &splitted = allData.split(QRegularExpression(R"([\r\n]+)"));

        for (const QString &line: splitted) {

            QRegularExpressionMatch match = re.match(line);

            if (match.hasMatch()) {
                const QString &tag = match.captured("tag");
                const QString &content = match.captured("content");

                if ("Title" == tag || "Name" == tag) {
                    title = content;
                } else if ("Version" == tag) {
                    version = content;
                } else if ("Author" == tag || "Credits" == tag) {
                    author = content;
                } else if ("Description" == tag) {
                    description = content;
                }
                if (!title.isEmpty() && !version.isEmpty() &&
                    !author.isEmpty() && !description.isEmpty()) {

                    break;
                }
            }
        }


        QString finalDesc = description.isEmpty() ? "[" + tr("No description") + "]" : description;
        QString finalAuth = author.isEmpty() ? "[" + tr("Unknown Author") + "]" : author;
        QString finalTitle = title.isEmpty() ? "[" + tr("Unknown Title") + "]" : title;

        QString finalVer = version.isEmpty() ? "[" + tr("Unknown Version") + "]" : version;
        ItemData::ItemStatus backupStatus = checkBackupStatus(addonName);
        auto foundNetData = std::find_if(esoSiteList.begin(), esoSiteList.end(), [&addonName](const QJsonObject &o) {

            return o.value("UIDir").toArray()[0] == addonName;
        });
        if (foundNetData != esoSiteList.end()) {
            const QString &extUid = foundNetData->value("UID").toString();
            auto *retData = new ItemData(cleanColorizers(finalAuth), cleanColorizers(finalTitle),
                                         finalVer, fPath,
                                         cleanColorizers(finalDesc),
                                         backupStatus,
                                         foundNetData->value("UIDownloadTotal").toString("0"),
                                         foundNetData->value("UIDownloadMonthly").toString("0"),
                                         foundNetData->value("UIFavoriteTotal").toString("0"),
                                         foundNetData->value("UIFileInfoURL").toString(),
                                         foundNetData->value("UIVersion").toString(),
                                         extUid);
            return retData;
        }
        return nullptr;
    }
}

void QAddonListModel::refreshFolderList() {

    int totalCount = addonList.count();
    if (!addonList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, totalCount - 1);
        addonList.clear();
        endRemoveRows();
    }
    QDir dir = QDir(addonFolderPath);
    const QFileInfoList &dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    int total = dirList.count();

    for (int i = 0; i < total; i++) {
        emit percent(i, total, tr("Refresh"));
        const QString &addonName = dirList.at(i).fileName();
        ItemData *rData = prepareAndFillDataByAddonName(addonName);
        if (!rData) {
            continue;
        }
        beginInsertRows(QModelIndex(), addonList.count(), addonList.count());
        addonList.append(*rData);
        endInsertRows();
        delete rData;

    }

    emit dataChanged(createIndex(0, 0), createIndex(addonList.count() - 1, 0));
    emit percent(total, total, "");
    emit addonsListChanged();

}

const QString &QAddonListModel::cleanColorizers(QString &input) {
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
    }
}

void QAddonListModel::refreshFromExternal() {
    refreshFromSiteList();
}

void QAddonListModel::refreshFromSiteList() {

    esoSiteDescriptionsCache.clear();

    if (!addonList.isEmpty()) {
        int totalCount = addonList.count();
        beginRemoveRows(QModelIndex(), 0, totalCount - 1);
        addonList.clear();
        endRemoveRows();
    }

    int total = esoSiteList.count() - 1;
    emit percent(0, total, tr("Updating"));

    int i = 0;

    for (const QJsonObject &findNow: esoSiteList) {
        i++;
        emit percent(i, total, tr("Updating"));
        const QString &catId = findNow.value("UICATID").toString();
        if (restrictedCategs.contains(catId)) {
            continue;
        }

        const QString &addonName = findNow.value("UIDir").toArray()[0].toString();
        const QString &fPath = addonFolderPath + QDir::separator() + addonName + QDir::separator() + addonName + ".txt";

        if (QFile(fPath).exists()) {
            continue;
        }

        const QString &extUid = findNow.value("UID").toString();

        beginInsertRows(QModelIndex(), addonList.count(), addonList.count());
        addonList.append(ItemData(findNow.value("UIAuthorName").toString(),
                                  findNow.value("UIName").toString(),
                                  findNow.value("UIVersion").toString(),
                                  fPath,
                                  QString(),
                                  ItemData::NotInstalled,
                                  findNow.value("UIDownloadTotal").toString("0"),
                                  findNow.value("UIDownloadMonthly").toString("0"),
                                  findNow.value("UIFavoriteTotal").toString("0"),
                                  findNow.value("UIFileInfoURL").toString(),
                                  findNow.value("UIVersion").toString(),
                                  extUid));
        endInsertRows();


    }
    emit percent(total, total, "");
    emit dataChanged(createIndex(0, 0), createIndex(addonList.count() - 1, 0));
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


    QMessageBox::StandardButton button = QMessageBox::warning(view, tr("Info"),
                                                              tr("Do you really want to delete this addon: %1?")
                                                                      .arg(aTitle),
                                                              QMessageBox::StandardButtons(
                                                                      QMessageBox::Yes | QMessageBox::No));
    if (button == QMessageBox::Yes) {
        const QString &parPath = QFileInfo(aPath).absolutePath();
        QDir(parPath).removeRecursively();

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
    const QDir &destDir = QDir(((useTar || useZip) ? QDir::tempPath() :
                                backupPath) + QDir::separator() + srcDir.dirName());

    prepareAndCleanDestDir(destDir);

    copyPath(srcDir.absolutePath(), destDir.absolutePath());

    if (useZip || useTar) {

        QProcess proc;
        proc.setWorkingDirectory(QDir::tempPath());
        if (useTar) {
            // имя архива. Пробелы убираем
            const QString &newTarCommand = QString(tarCommand)
                    .arg(destDir.dirName().replace(QRegularExpression(R"(\s+)"), ""));
            QStringList commandList = newTarCommand.trimmed().split(QRegularExpression(R"(\s+)"));

            const QString &command = commandList.value(0);
            commandList.removeAt(0);
            const QString &dDir = destDir.dirName();
            commandList << (!dDir.contains(" ") ? dDir : R"(")" + dDir + R"(")");

            proc.start(command, commandList);
            proc.waitForFinished();
            QProcess::ProcessError res = proc.error();
            const QString &strRes = QString(proc.readAllStandardOutput());
            const QString &errRes = QString(proc.readAllStandardError());

        } else if (useZip) {
            // имя архива. Пробелы убираем
            const QString &newZipCommand = QString(zipCommand)
                    .arg(destDir.dirName().replace(QRegularExpression(R"(\s+)"), ""));
            QStringList commandList = newZipCommand.trimmed().split(QRegularExpression(R"(\s+)"));

            const QString &command = commandList.value(0);
            commandList.removeAt(0);
            const QString &dDir = destDir.dirName();
            commandList << (!dDir.contains(" ") ? dDir : R"(")" + dDir + R"(")");

            proc.start(command, commandList);
            proc.waitForFinished();
            QProcess::ProcessError res = proc.error();
            const QString &strRes = QString(proc.readAllStandardOutput());
            const QString &errRes = QString(proc.readAllStandardError());


        }

        QDir ddir(destDir);
        ddir.removeRecursively();
        const QString &ext = (useTar ? ".tgz" : (useZip ? ".zip" : ""));

        const QString &srcPath = QDir::tempPath() + QDir::separator() + ddir.dirName() + ext;
        const QString &dstPath = backupPath + QDir::separator() + ddir.dirName() + ext;

        if (QFile::exists(dstPath)) {
            QFile::remove(dstPath);
        }
        QFile::copy(srcPath, dstPath);
        QFile::remove(srcPath);
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
            value = headerTitle;
    }
    return value;
}

void QAddonListModel::setModelData(const PreferencesType &data) {

    addonFolderPath = data.value("addonFolderPath").toString();
    backupPath = data.value("backupPath").toString();

    useTar = data.value("useTar").toBool();
    useZip = data.value("useZip").toBool();

    tarCommand = data.value("tarCommand").toString();
    zipCommand = data.value("zipCommand").toString();
    zipExtractCommand = data.value("zipExtractCommand").toString();

}

ItemData::ItemStatus QAddonListModel::checkBackupStatus(const QString &aName) const {

    if (QFile(backupPath + QDir::separator() + aName + ".tgz").exists() ||
        QFile(backupPath + QDir::separator() + aName + ".zip").exists() ||
        QDir(backupPath + QDir::separator() + aName).exists()) {
        return ItemData::InstalledBackedUp;
    } else {
        return ItemData::Installed;
    }

}

void QAddonListModel::onReportError(QNetworkReply *reply) {
    emit percent(100, 100);
    QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                          reply->errorString());
}

void QAddonListModel::onReportSuccess(const QByteArray &res, QNetworkReply *reply) {

    disconnect(bdl, &BinaryDownloader::reportSuccess, this, &QAddonListModel::onReportSuccess);
    disconnect(bdl, &BinaryDownloader::reportError, this, &QAddonListModel::onReportError);
    if (!res.isEmpty()) {
        const QJsonDocument &document = QJsonDocument::fromJson(res);
        if (!document.isEmpty() && document.isArray()) {
            const QJsonArray &dataArray = document.array();
            emit percent(0, 100, tr("Processing downloaded data"));
            int total = dataArray.size();
            int i = 0;

            for (QJsonValue v: dataArray) {
                if (v.isObject()) {

                    const QJsonObject &findNow = v.toObject();

                    esoSiteList.append(findNow);
                }
                i++;
                emit percent(i, total, tr("Processing downloaded data"));
            }
            refreshFolderList();
        } else {
            QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                                  tr("Invalid data"));
        }
    }
}

void QAddonListModel::refreshESOSiteList() {

    connect(bdl, &BinaryDownloader::reportSuccess, this, &QAddonListModel::onReportSuccess);
    connect(bdl, &BinaryDownloader::reportError, this, &QAddonListModel::onReportError);
    emit percent(0, 100, tr("Updating data"));
    QNetworkReply *cRep = bdl->start();
    if (cRep) {
        connect(cRep, &QNetworkReply::downloadProgress, this, &QAddonListModel::onPercentDownload);
    }
}

void QAddonListModel::setTopIndex() {
    auto *view = qobject_cast<QTreeView *>(parent());
    const QModelIndex &index = view->model()->index(0, 0);
    if (index.isValid()) {
        view->setCurrentIndex(index);
    }
}

void QAddonListModel::reinstallAddonClicked() {
    QTreeView *view = qobject_cast<QTreeView *>(parent());
    const QModelIndexList &selectedSet = view->selectionModel()->selectedIndexes();
    if (selectedSet.count() > 1) {
        return; // fuckup
    }

    const QModelIndex &index = selectedSet[0];
    if (index.isValid()) {

        const QString &aTitle = index.data(Qt::DisplayRole).toString();
        ItemData::ItemStatus aStatus = index.data(QAddonListModel::StatusRole).value<ItemData::ItemStatus>();
        const QString &toDo = (aStatus == ItemData::NotInstalled ? tr("install") : tr("update or reinstall"));
        QMessageBox::StandardButton button = QMessageBox::warning(view, tr("Info"),
                                                                  tr("Do you really want to %1 this addon: %2?")
                                                                          .arg(toDo, aTitle),
                                                                  QMessageBox::StandardButtons(
                                                                          QMessageBox::Yes | QMessageBox::No));

        if (button == QMessageBox::No) {
            return;
        }

        QRegExp rx(R"(info([0-9]*)-([^\.]+)\.html)");
        QString &&urlPath = index.data(QAddonListModel::FileInfoURLRole).toString();
        QString &downPath = urlPath.replace(rx, R"(dl\1/\2.zip)");


        const QString &aPath = index.data(QAddonListModel::PathRole).toString();
        const QString &tmpDirPath = QDir::tempPath() + QDir::separator() + QDir(
                QFileInfo(aPath).absolutePath()
        ).dirName();
        const QString &tmpFilePath = tmpDirPath + ".zip";

        QFile *file = new QFile(tmpFilePath);

        if (!file->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
            emit percent(100, 100, "");
            QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                                  tr("I/O error"));
            return;
        }

        auto *tmpRedirectManager = new QNetworkAccessManager(this);
        connect(tmpRedirectManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {

            QNetworkReply::NetworkError error = reply->error();

            if (error == QNetworkReply::NetworkError::NoError) {

                file->flush();
                file->close();

                QProcess extractProc;
                extractProc.setWorkingDirectory(QDir::tempPath());

                QStringList commandList = zipExtractCommand.trimmed().split(
                        QRegularExpression(R"(\s+)")
                );

                const QString &command = commandList.value(0);
                commandList.removeAt(0);
                commandList << QFileInfo(tmpFilePath).fileName();

                extractProc.start(command, commandList);
                extractProc.waitForFinished();
                QProcess::ProcessError res = extractProc.error();
                const QString &strRes = QString(extractProc.readAllStandardOutput());
                const QString &errRes = QString(extractProc.readAllStandardError());

                QFile::remove(tmpFilePath);

                QDir &&srcDir = QDir(tmpDirPath);
                const QString &addonName = srcDir.dirName();
                const QDir &dstDir = QDir(addonFolderPath + QDir::separator() + addonName);

                prepareAndCleanDestDir(dstDir);
                copyPath(srcDir.absolutePath(), dstDir.absolutePath());
                srcDir.removeRecursively();

                const QString &aPath = addonFolderPath + QDir::separator() + addonName +
                                       QDir::separator() + addonName + ".txt";
                emit percent(100, 100);
                emit backToInstalled(aPath);

            } else {
                if (file) {
                    file->close();
                    file->remove();
                }
                emit percent(100, 100);
                QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                                      reply->errorString());
            }
            reply->close();
            reply->deleteLater();
            tmpRedirectManager->deleteLater();
            delete file;
        });
        emit percent(0, 100, tr("Downloading from site"));
        QNetworkRequest &&request = QNetworkRequest(QUrl(downPath));
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        QNetworkReply *cRep = tmpRedirectManager->get(request);
        connect(cRep, &QNetworkReply::readyRead, this, [=]() {
            if (file && file->isOpen()) {
                file->write(cRep->readAll());
            }
        });
        connect(cRep, &QNetworkReply::downloadProgress, this, &QAddonListModel::onPercentDownload);
    }
}

void QAddonListModel::prepareAndCleanDestDir(const QDir &destDir) const {
    if (!destDir.exists()) {
        destDir.mkpath(".");
    } else if (!destDir.isEmpty()) {

        for (const QFileInfo &c: destDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files)) {
            if (c.isDir()) {
                QDir(c.absoluteFilePath()).removeRecursively();
            } else if (c.isFile()) {
                QFile(c.absoluteFilePath()).remove();
            }
        }
    }
}

void QAddonListModel::onPercentDownload(qint64 c, qint64 t) {
    if (!t) {
        emit percent(100, 100);
    } else {
        emit percent(c, t, tr("Downloading from site"));
    }
}

void QAddonListModel::setHeaderTitle(const QString &hTitle) {
    QAddonListModel::headerTitle = hTitle;
    emit headerDataChanged(Qt::Horizontal, 0, 0);
}

void QAddonListModel::disconnectWatcher() {
    if (qsw) {
        disconnect(qsw, &QFileSystemWatcher::directoryChanged,
                   this, &QAddonListModel::refresh);
        qsw->removePath(addonFolderPath);
        qsw->removePath(backupPath);
        delete qsw;
        qsw = nullptr;
    }
}

void QAddonListModel::connectWatcher() {
    if (!qsw) {
        qsw = new QFileSystemWatcher(QStringList() << this->addonFolderPath << this->backupPath);
        connect(qsw, &QFileSystemWatcher::directoryChanged,
                this, &QAddonListModel::refresh);
    }
}

QString QAddonListModel::tryToGetExtraData(const QString &UID, const QByteArray &contentType) {

    const QString &tmp = esoSiteDescriptionsCache.value(UID);

    if (tmp.isEmpty()) {
        bdl->setDownloadUrl(detailsUrl + UID + ".json");
        bdl->setContentType(contentType);

        QEventLoop loop;
        QNetworkReply *cRep = bdl->start();
        connect(cRep, &QNetworkReply::downloadProgress,
                this, &QAddonListModel::onPercentDownload);
        connect(cRep, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec(QEventLoop::ExcludeUserInputEvents);
        QByteArray retRes = bdl->getRequestResult();
        if (!retRes.isEmpty()) {
            const QJsonDocument &document = QJsonDocument::fromJson(retRes);
            if (!document.isEmpty() && document.isArray()) {
                const QJsonArray &dataArray = document.array();
                if (!dataArray.isEmpty()) {
                    QString UIdesc = dataArray.at(0).toObject().value("UIDescription").toString();

                    const QString &finalDesc = UIdesc.replace(
                            QRegularExpression(R"(\[[^\]]+\])"),"");
                    esoSiteDescriptionsCache.insert(UID, finalDesc);
                    return finalDesc;
                }
                return {};
            }
            return {};
        }
        return retRes;
    } else {
        return tmp;
    }
}






