//
// Created by esorochinskiy on 09.12.22.
//

#ifndef LINION_BINARYDOWNLOADER_H
#define LINION_BINARYDOWNLOADER_H


#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class BinaryDownloader : public QObject {
Q_OBJECT
public:
    explicit BinaryDownloader(const QString &urlName = QString(),
                              const QByteArray &contentType = "application/json",
                              QObject *parent = nullptr);
    ~BinaryDownloader() override;

    void setDownloadUrl(const QString &urlName);
    QNetworkReply *start();

    [[nodiscard]]
    const QByteArray &getRequestResult() const;

private:
    QNetworkAccessManager *manager  {nullptr};
    QMap<QUrl, QByteArray> m_buffers;
    QByteArray requestResult;
    QNetworkRequest *request {nullptr};
    QByteArray contentType;
public:
    void setContentType(const QByteArray &cType);

signals:
    void reportSuccess(const QByteArray &, QNetworkReply *);
    void reportError(QNetworkReply *);


private slots:
    void replyFinished(QNetworkReply *replyFinished);
};


#endif //LINION_BINARYDOWNLOADER_H
