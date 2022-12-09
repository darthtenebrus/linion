//
// Created by esorochinskiy on 09.12.22.
//

#include "BinaryDownloader.h"

BinaryDownloader::BinaryDownloader(const QString &urlName, QObject *parent) : QObject(parent),
    manager(new QNetworkAccessManager(this)) {
    m_buffer.clear();
    connect(manager, &QNetworkAccessManager::finished, this, &BinaryDownloader::replyFinished);
    setDownloadUrl(urlName);
}

BinaryDownloader::~BinaryDownloader() {
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
    m_currentReply = manager->get(request);
    connect(m_currentReply, &QNetworkReply::readyRead, this, [=]() {
        m_buffer += m_currentReply->readAll();
    });
    return m_currentReply;
}

void BinaryDownloader::setDownloadUrl(const QString &urlName) {
    request = QNetworkRequest(QUrl(urlName));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setRawHeader("Content-Type", "application/json");
}

