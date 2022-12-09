//
// Created by esorochinskiy on 09.12.22.
//

#include "BinaryDownloader.h"

BinaryDownloader::BinaryDownloader(const QString &urlName, QObject *parent) : QObject(parent),
    manager(new QNetworkAccessManager(this)) {
    connect(manager, &QNetworkAccessManager::finished, this, &BinaryDownloader::replyFinished);
    setDownloadUrl(urlName);
}

BinaryDownloader::~BinaryDownloader() {
    delete request;
    delete manager;
}

void BinaryDownloader::replyFinished(QNetworkReply *replyFinished) {

    QNetworkReply::NetworkError error = replyFinished->error();
    if (error == QNetworkReply::NetworkError::NoError) {
        if (!m_buffer.isEmpty()) {
            emit reportSuccess(m_buffer, replyFinished);
        } else {
            emit reportError(replyFinished);
        }
    } else {
        emit reportError(replyFinished);
    }
    replyFinished->close();
    replyFinished->deleteLater();
}

QNetworkReply *BinaryDownloader::start() {
    if (request) {
        m_buffer.clear();
        QNetworkReply *currentReply = manager->get(*request);
        connect(currentReply, &QNetworkReply::readyRead, this, [=]() {
            m_buffer += qobject_cast<QNetworkReply *>(sender())->readAll();
        });
        return currentReply;
    }
    return nullptr;
}

void BinaryDownloader::setDownloadUrl(const QString &urlName) {

    if (request) {
        delete request;
        request = nullptr;
    }

    if (!urlName.isEmpty()) {
        request = new QNetworkRequest(QUrl(urlName));
        request->setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        request->setRawHeader("Content-Type", "application/json");
    }
}

