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
    connectWatcher();
    connect(manager, &QNetworkAccessManager::finished, this, &QAddonListModel::replyFinished);

}

QAddonListModel::~QAddonListModel() {
    disconnectWatcher();
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
        case Qt::DecorationRole:
            value = (addonList.at(index.row()).isStatus() == ItemData::InstalledBackedUp ?
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
        case QAddonListModel::StatusRole:
            value = addonList.at(index.row()).isStatus();
            break;
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
#ifdef _DEBUG
        qDebug() << splitted;
#endif
        for (const QString &line : splitted) {

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
        auto foundNetData = std::find_if(esoSiteList.begin(), esoSiteList.end(), [&addonName](QJsonObject o) {

            return o.value("UIDir").toArray()[0] == addonName;
        });
        if (foundNetData != esoSiteList.end()) {
            auto *retData = new ItemData(cleanColorizers(finalAuth), cleanColorizers(finalTitle),
                                         finalVer, fPath,
                                         cleanColorizers(finalDesc),
                                         backupStatus,
                                         foundNetData->value("UIDownloadTotal").toString("0"),
                                         foundNetData->value("UIDownloadMonthly").toString("0"),
                                         foundNetData->value("UIFavoriteTotal").toString("0"),
                                         foundNetData->value("UIFileInfoURL").toString(),
                                         foundNetData->value("UIVersion").toString());
            return retData;
        }
        return nullptr;
    }
}

void QAddonListModel::refreshFolderList() {

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
        ItemData *rData = prepareAndFillDataByAddonName(addonName);
        if (!rData) {
            continue;
        }
        addonList.append(*rData);
        delete rData;

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

void QAddonListModel::refreshFromExternal() {
    refreshFromSiteList();
    setTopIndex();
}

void QAddonListModel::refreshFromSiteList() {
#ifdef _DEBUG
    qDebug() << "Refresh from ESO Site";
#endif
    int totalCount = addonList.count();
    beginRemoveRows(QModelIndex(), 0, totalCount >= 1 ? totalCount - 1 : 0);
    addonList.clear();
    endRemoveRows();

    int total = esoSiteList.count() - 1;
    emit percent(0, total, tr("Updating"));
    beginInsertRows(QModelIndex(), 0, total);
    int i = 0;
    for (const QJsonObject &findNow: esoSiteList) {
        i++;
        emit percent(i, total, tr("Updating"));
        const QString &addonName = findNow.value("UIDir").toArray()[0].toString();
        const QString &fPath = addonFolderPath + QDir::separator() + addonName + QDir::separator() + addonName + ".txt";

        if (QFile(fPath).exists()) {
            continue;
        }
        const QJsonArray &thumbs = findNow.value("UIIMG_Thumbs").toArray();

        addonList.append(ItemData(findNow.value("UIAuthorName").toString(),
                                  findNow.value("UIName").toString(),
                                  findNow.value("UIVersion").toString(),
                                  fPath,
                                  QString(), // todo: external URL
                                  ItemData::NotInstalled,
                                  findNow.value("UIDownloadTotal").toString("0"),
                                  findNow.value("UIDownloadMonthly").toString("0"),
                                  findNow.value("UIFavoriteTotal").toString("0"),
                                  findNow.value("UIFileInfoURL").toString(),
                                  findNow.value("UIVersion").toString(),
                                  !thumbs.isEmpty() ? thumbs[0].toString() : ""));

    }
    endInsertRows();
    emit dataChanged(createIndex(0, 0), createIndex(addonList.count() - 1, 0));
    emit percent(total, total, "");
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

void QAddonListModel::sort(int column, Qt::SortOrder order) {


    emit layoutAboutToBeChanged();
    qSort(addonList.begin(), addonList.end(), [&order](ItemData &v1, ItemData &v2) {

        const QString &ver1 = v1.getVersion();
        const QString &site1 = v1.getSiteVersion();

        const QString &ver2 = v2.getVersion();
        const QString &site2 = v2.getSiteVersion();

        bool preCondition = ((ver1 != site1) && (ver2 == site2));
        bool postCondition = ((ver1 == site1) && (ver2 != site2));
#ifdef _DEBUG
        qDebug() << "1 title = " + v1.getAddonTitle();
        qDebug() << "2 title = " + v2.getAddonTitle();

        qDebug() << "pre = " << preCondition;
        qDebug() << "post = " << postCondition;
#endif

        if (preCondition) {
            return preCondition;
        }

        if (postCondition) {
            return false;
        }

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

void QAddonListModel::replyFinished(QNetworkReply *reply) {

    emit percent(100, 100);
    QNetworkReply::NetworkError error = reply->error();
    if (error == QNetworkReply::NetworkError::NoError) {

        if (!m_buffer.isEmpty()) {
            const QJsonDocument &document = QJsonDocument::fromJson(m_buffer);
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
    reply->deleteLater();
}

void QAddonListModel::refreshESOSiteList() {
    m_buffer.clear();
    emit percent(0, 100, tr("Updating data"));
    QNetworkRequest request = QNetworkRequest(QUrl(listUrl));
    request.setRawHeader("Content-Type", "application/json");
    m_currentReply = manager->get(QNetworkRequest(request));
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &QAddonListModel::onPercentDownload);
    connect(m_currentReply, &QNetworkReply::readyRead, this, [=]() {
        m_buffer += m_currentReply->readAll();
    });
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

        const ItemData::ItemStatus &cStatus = index.data(QAddonListModel::StatusRole).value<ItemData::ItemStatus>();
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

                emit percent(100, 100);
                //emit refreshSelf();
                if (cStatus != ItemData::NotInstalled) {
                    ItemData *rData = prepareAndFillDataByAddonName(addonName);
                    if (rData) {
                        addonList.replace(index.row(), *rData);
                        delete rData;
                        emit dataChanged(index, index);
                        emit currentRowDetailChanged(index, index);
                    }
                } else {
                    emit backToInstalled(true);
                }
                connect(qsw, &QFileSystemWatcher::directoryChanged,
                        this, &QAddonListModel::refresh);
            } else {
                if (file) {
                    file->close();
                    file->remove();
                }
                emit percent(100, 100);
                QMessageBox::critical(qobject_cast<QTreeView *>(parent()), tr("Fatal"),
                                      reply->errorString());
            }
            reply->deleteLater();
            delete file;
        });
        emit percent(0, 100, tr("Downloading from site"));
        QNetworkRequest &&request = QNetworkRequest(QUrl(downPath));
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
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
    disconnect(qsw, &QFileSystemWatcher::directoryChanged,
               this, &QAddonListModel::refresh);
    qsw->removePath(addonFolderPath);
    qsw->removePath(backupPath);
    delete qsw;
    qsw = nullptr;
}

void QAddonListModel::connectWatcher() {
    if (!qsw) {
        qsw = new QFileSystemWatcher(QStringList() << this->addonFolderPath << this->backupPath);
        connect(qsw, &QFileSystemWatcher::directoryChanged,
                this, &QAddonListModel::refresh);
    }
}




