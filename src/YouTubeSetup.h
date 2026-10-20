#pragma once
#include <QObject>
#include <QByteArray>
#include <memory>
class QNetworkAccessManager;
class QNetworkReply;
class QSaveFile;
class QCryptographicHash;
class QTemporaryDir;
class QProcess;

class YouTubeSetup final : public QObject {
    Q_OBJECT
public:
    explicit YouTubeSetup(QObject* parent=nullptr,const QString& installRoot={});
    ~YouTubeSetup() override;
    void start();
    void cancel();
    bool busy() const {return busy_;}
    static QString toolsPath();
    static bool ready();
signals:
    void progress(const QString& message);
    void finished(bool success,const QString& message);
private:
    void downloadNext();void extract();void install();void fail(const QString& message);
    QNetworkAccessManager* network_;
    QNetworkReply* reply_=nullptr;
    QProcess* extractor_;
    std::unique_ptr<QTemporaryDir> staging_;
    std::unique_ptr<QSaveFile> file_;
    std::unique_ptr<QCryptographicHash> hash_;
    QString ioError_;
    bool busy_=false;
    int step_=0;
    qint64 received_=0;
    QString destination_;
};
