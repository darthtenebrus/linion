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
#include <QFileSystemWatcher>
#include <QTreeView>
#include <QProcess>


QAddonListModel::QAddonListModel(const QHash<QString, QVariant> &settings, QObject *parent)
        : QAbstractListModel(parent) {

    setModelData(settings);
    qsw = new QFileSystemWatcher(QStringList() << this->addonFolderPath);
    connect(qsw, &QFileSystemWatcher::directoryChanged, this, &QAddonListModel::refresh);

}

QAddonListModel::~QAddonListModel() {
    qsw->removePath(addonFolderPath);
    delete qsw;
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
            value = QPixmap(addonList.at(index.row()).isStatus() ?
                            ":/images/green_check.png" : ":/images/red_cross.png");
        default:
            break;
    }

    return value;
}

void QAddonListModel::refreshFolderList() {
    QRegularExpression re(R"(##\s+(?<tag>[A-Za-z]+):\s+(?<content>.*))");
    int totalCount = addonList.count();
    beginRemoveRows(QModelIndex(), 0, totalCount >=1 ? totalCount - 1 : 0);
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
#ifdef _DEBUG
            qDebug() << title << " " << version;
#endif
            if (!title.isEmpty() && !version.isEmpty() && !author.isEmpty()) {

                const QString &finalDesc = description.isEmpty() ? "[" + tr("No description") + "]" : description;
                addonList.append(ItemData(cleanColorizers(author), cleanColorizers(title),
                                          version, fPath,
                                          finalDesc,
                                          true));
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
    refreshFolderList();
    QModelIndex index = this->index(0, 0);
    if (index.isValid()) {
        auto *view = qobject_cast<QTreeView *>(parent());
        view->setCurrentIndex(index);
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
    for(int i = 0; i < total; i++) {
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

    if(useZip || useTar) {
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

    foreach (QString f, srcDir.entryList(QDir::Files)) {
            QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }

    foreach (QString d, srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
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
        return order == Qt::AscendingOrder ? v1.getAddonTitle() < v2.getAddonTitle() : v1.getAddonTitle() > v2.getAddonTitle();
    });
    emit layoutChanged();

}

void QAddonListModel::setModelData(const QHash<QString, QVariant> &data) {

    addonFolderPath = data.value("addonFolderPath").toString();
    backupPath = data.value("backupPath").toString();

    useTar = data.value("useTar").toBool();
    useZip = data.value("useZip").toBool();

    tarCommand = data.value("tarCommand").toString();
    zipCommand = data.value("zipCommand").toString();

}


