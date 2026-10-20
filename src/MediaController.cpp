#include "MediaController.h"
#include "ImportJob.h"
#include <QApplication>
#include <QProcess>
#include <QWidget>
#include <QVideoWidget>
#include <QVideoSink>
#include <QVideoFrame>
#include <QAudioOutput>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace {
void report(const QJsonObject& object){
 const auto line=QJsonDocument(object).toJson(QJsonDocument::Compact)+"\n";
 DWORD written=0;WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),line.constData(),DWORD(line.size()),&written,nullptr);
}
}
MediaController::MediaController(QObject* parent):QObject(parent),process_(new QProcess(this)){
 connect(process_,&QProcess::started,this,[this]{job_=std::make_unique<ImportJob>();job_->attach(process_->processId());});
 connect(process_,&QProcess::readyReadStandardOutput,this,[this]{
  buffer_+=process_->readAllStandardOutput();
  while(buffer_.contains('\n')){
   const auto end=buffer_.indexOf('\n');const auto line=buffer_.left(end);buffer_.remove(0,end+1);
   const auto object=QJsonDocument::fromJson(line).object();
   if(object.contains("frame"))emit frameReady();
   if(object.contains("error")){error_=QMediaPlayer::ResourceError;errorText_=object["error"].toString();emit errorOccurred(error_,errorText_);}
  }
  if(buffer_.size()>65536)buffer_.clear();
 });
 // Drain diagnostic output so decoder errors cannot accumulate in the UI process.
 connect(process_,&QProcess::readyReadStandardError,this,[this]{process_->readAllStandardError();});
 connect(process_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError){if(state_!=QMediaPlayer::StoppedState){error_=QMediaPlayer::ResourceError;errorText_="Video worker could not run: "+process_->errorString();emit errorOccurred(error_,errorText_);}});
 connect(process_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int,QProcess::ExitStatus){if(state_!=QMediaPlayer::StoppedState){error_=QMediaPlayer::ResourceError;errorText_="Video playback process stopped unexpectedly.";emit errorOccurred(error_,errorText_);}});
}
MediaController::~MediaController(){stop();}
void MediaController::send(const QByteArray& command){if(process_->state()==QProcess::Running)process_->write(command+"\n");}
void MediaController::setVideoOutput(QWidget* widget){output_=widget;if(widget)send("PARENT "+QByteArray::number(qulonglong(widget->winId())));}
void MediaController::setVolume(float volume){volume_=volume;send("VOLUME "+QByteArray::number(volume));}
void MediaController::play(){
 if(source_.isEmpty() || !output_)return;
 if(process_->state()!=QProcess::NotRunning){state_=QMediaPlayer::PlayingState;send("PLAY");return;}
 error_=QMediaPlayer::NoError;errorText_.clear();buffer_.clear();state_=QMediaPlayer::PlayingState;
 process_->start(QCoreApplication::applicationFilePath(),{"--media-worker",QString::number(qulonglong(output_->winId())),source_.toLocalFile(),QString::number(volume_)});
}
void MediaController::pause(){state_=QMediaPlayer::PausedState;send("PAUSE");}
void MediaController::stop(){
 state_=QMediaPlayer::StoppedState;
 if(process_->state()!=QProcess::NotRunning){process_->kill();process_->waitForFinished(1000);}
}
int runMediaWorker(QApplication& app){
 const auto args=app.arguments();if(args.size()<4)return 2;
 HWND parent=reinterpret_cast<HWND>(args.at(2).toULongLong());if(!IsWindow(parent))return 3;
 QVideoWidget video;video.setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowDoesNotAcceptFocus);
 video.setAspectRatioMode(Qt::KeepAspectRatioByExpanding);video.show();
 const auto handle=reinterpret_cast<HWND>(video.winId());
 auto embed=[&](HWND target){
  if(!IsWindow(target))return;
  ShowWindow(handle,SW_HIDE);
  auto style=GetWindowLongPtrW(handle,GWL_STYLE);SetWindowLongPtrW(handle,GWL_STYLE,(style&~WS_POPUP)|WS_CHILD);
  SetLastError(0);SetParent(handle,target);if(GetLastError()!=0){report({{"error","Could not embed video surface."}});return;}
  parent=target;
 };
 embed(parent);
 QMediaPlayer player;QAudioOutput audio;audio.setVolume(args.size()>4?args.at(4).toFloat():0);
 player.setAudioOutput(&audio);player.setVideoOutput(&video);player.setLoops(QMediaPlayer::Infinite);
 QElapsedTimer clock;clock.start();qint64 last=-1000;
 QObject::connect(video.videoSink(),&QVideoSink::videoFrameChanged,&app,[&](const QVideoFrame& frame){if(frame.isValid() && clock.elapsed()-last>=250){last=clock.elapsed();report({{"frame",true}});}});
 QObject::connect(&player,&QMediaPlayer::errorOccurred,&app,[&](QMediaPlayer::Error,const QString& message){report({{"error",message}});});
 QTimer poll;QByteArray commands;
 QObject::connect(&poll,&QTimer::timeout,&app,[&]{
  if(!IsWindow(parent)){app.quit();return;}
  RECT rect{};GetClientRect(parent,&rect);
  RECT current{};GetWindowRect(handle,&current);
  if(current.right-current.left!=rect.right || current.bottom-current.top!=rect.bottom || !IsWindowVisible(handle))
   SetWindowPos(handle,HWND_BOTTOM,0,0,rect.right,rect.bottom,SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_FRAMECHANGED);
  DWORD available=0;const auto input=GetStdHandle(STD_INPUT_HANDLE);
  if(PeekNamedPipe(input,nullptr,0,nullptr,&available,nullptr) && available){
   char bytes[4096];DWORD read=0;if(ReadFile(input,bytes,qMin<DWORD>(available,sizeof(bytes)),&read,nullptr))commands.append(bytes,int(read));
  }
  while(commands.contains('\n')){
   const auto end=commands.indexOf('\n');const auto command=commands.left(end);commands.remove(0,end+1);
   if(command=="PLAY")player.play();else if(command=="PAUSE")player.pause();
   else if(command.startsWith("PARENT "))embed(reinterpret_cast<HWND>(command.mid(7).toULongLong()));
   else if(command.startsWith("VOLUME "))audio.setVolume(command.mid(7).toFloat());
  }
 });poll.start(100);
 player.setSource(QUrl::fromLocalFile(args.at(3)));player.play();
 return app.exec();
}
