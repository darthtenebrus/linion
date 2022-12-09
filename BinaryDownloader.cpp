//
// Created by esorochinskiy on 09.12.22.
//

#include "BinaryDownloader.h"

BinaryDownloader::BinaryDownloader(const QString &urlName,
                                   const QByteArray &contentType,
                                   QObject *parent) : QObject(parent),
                                   contentType(contentType),
    manager(new QNetworkAccessManager(this)) {
    connect(manager, &QNetworkAccessManager::finished, this, &BinaryDownloader::replyFinished);
    setDownloadUrl(urlName);
}

BinaryDownloader::~BinaryDownloader() {
    if (request) {
        delete request;
    }
    delete manager;
}

void BinaryDownloader::replyFinished(QNetworkReply *replyFinished) {
    QNetworkReply::NetworkError error = replyFinished->error();
    if (error == QNetworkReply::NetworkError::NoError) {
        const QByteArray &mb = m_buffers.value(replyFinished->url());
        if (!mb.isEmpty()) {
            emit reportSuccess(mb, replyFinished);
        } else {
            emit reportError(replyFinished);
        }
    } else {
        emit reportError(replyFinished);
    }
    m_buffers.remove(replyFinished->url());
    replyFinished->close();
    replyFinished->deleteLater();
}

QNetworkReply *BinaryDownloader::start() {
    if (request) {
        m_buffers.insert(request->url(), QByteArray());
        QNetworkReply *currentReply = manager->get(*request);

        connect(currentReply, &QNetworkReply::readyRead, this, [=]() {
            auto *origin = qobject_cast<QNetworkReply *>(sender());
#ifdef _DEBUG
            qDebug() << origin->url();
#endif
            QByteArray mb = m_buffers.value(origin->url());
            mb += origin->readAll();
            m_buffers.insert(origin->url(), mb);
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
        request->setRawHeader("Content-Type", contentType);

    }
}

void BinaryDownloader::setContentType(const QByteArray &cType) {
    contentType = cType;
}


