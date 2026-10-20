#pragma once
#include <QObject>
#include <QMediaPlayer>
#include <QUrl>
#include <memory>
class ImportJob;
class QProcess;
class QWidget;
class QApplication;
int runMediaWorker(QApplication& app);
class MediaController:public QObject {
 Q_OBJECT
public:
 explicit MediaController(QObject* parent=nullptr);
 ~MediaController();
 void setVideoOutput(QWidget* widget);
 void setSource(const QUrl& source){source_=source;}
 QUrl source()const{return source_;}
 void play();void pause();void stop();
 QMediaPlayer::PlaybackState playbackState()const{return state_;}
 QMediaPlayer::Error error()const{return error_;}
 QString errorString()const{return errorText_;}
 void setVolume(float volume);
signals:
 void frameReady();
 void errorOccurred(QMediaPlayer::Error error,const QString& message);
private:
 void send(const QByteArray& command);
 std::unique_ptr<ImportJob> job_;
 QProcess* process_;QWidget* output_=nullptr;QUrl source_;QByteArray buffer_;
 QMediaPlayer::PlaybackState state_=QMediaPlayer::StoppedState;
 QMediaPlayer::Error error_=QMediaPlayer::NoError;QString errorText_;float volume_=0;
};
