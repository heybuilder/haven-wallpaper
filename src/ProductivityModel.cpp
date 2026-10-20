#include "ProductivityModel.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>
#include <QUuid>

ProductivityModel::ProductivityModel(QObject* parent,const QString& storage,std::function<qint64()> clock):QObject(parent),clock_(std::move(clock)) {
    elapsed_.start();
    path_=storage.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/intentions.json":storage;
    load();
    ticker_=new QTimer(this);ticker_->setInterval(250);
    connect(ticker_,&QTimer::timeout,this,&ProductivityModel::update);
}
ProductivityModel::~ProductivityModel(){save();}
void ProductivityModel::add(const QString& text,bool goal){
    const auto cleaned=text.trimmed().left(240);if(cleaned.isEmpty())return;
    items_.push_back({QUuid::createUuid().toString(QUuid::WithoutBraces),cleaned,false,goal});
    save();emit intentionsChanged();
}
void ProductivityModel::setDone(const QString& id,bool done){
    for(auto& item:items_)if(item.id==id){item.done=done;save();emit intentionsChanged();return;}
}
void ProductivityModel::remove(const QString& id){
    for(int i=0;i<items_.size();++i)if(items_[i].id==id){items_.removeAt(i);save();emit intentionsChanged();return;}
}
int ProductivityModel::count(bool goal,bool completed) const {
    int result=0;for(const auto& item:items_)if(item.goal==goal && (!completed || item.done))++result;return result;
}
void ProductivityModel::setTheme(const QString& theme){
    if(theme!="Sage morning" && theme!="Rose linen" && theme!="Lavender dusk")return;
    theme_=theme;save();emit themeChanged();
}
QString ProductivityModel::displayTime(qint64 ms){
    const qint64 seconds=(std::max<qint64>(0,ms)+999)/1000;
    if(seconds>=3600)return QString("%1:%2:%3").arg(seconds/3600,2,10,QChar('0')).arg((seconds/60)%60,2,10,QChar('0')).arg(seconds%60,2,10,QChar('0'));
    return QString("%1:%2").arg(seconds/60,2,10,QChar('0')).arg(seconds%60,2,10,QChar('0'));
}
QString ProductivityModel::phaseName() const {return phase_==0?"Focus gently":phase_==1?"A little breather":"A longer rest";}
qint64 ProductivityModel::phaseDuration() const {return qint64(phase_==0?focusMinutes_:phase_==1?shortMinutes_:longMinutes_)*60000;}
void ProductivityModel::setTimerMinutes(int minutes){timerMinutes_=qBound(1,minutes,180);timer_.reset(qint64(timerMinutes_)*60000);arm();save();emit clocksChanged();}
void ProductivityModel::setPomodoroMinutes(int focus,int shortBreak,int longBreak){
    focusMinutes_=qBound(1,focus,180);shortMinutes_=qBound(1,shortBreak,60);longMinutes_=qBound(1,longBreak,120);
    pomodoro_.reset(phaseDuration());arm();save();emit clocksChanged();
}
void ProductivityModel::toggleTimer(){if(timer_.running())timer_.pause(now());else{if(timer_.value(now())==0)timer_.reset(qint64(timerMinutes_)*60000);timer_.start(now());}arm();save();emit clocksChanged();}
void ProductivityModel::resetTimer(){timer_.reset(qint64(timerMinutes_)*60000);arm();save();emit clocksChanged();}
void ProductivityModel::toggleStopwatch(){if(stopwatch_.running())stopwatch_.pause(now());else stopwatch_.start(now());arm();save();emit clocksChanged();}
void ProductivityModel::resetStopwatch(){stopwatch_.reset(0);arm();save();emit clocksChanged();}
void ProductivityModel::togglePomodoro(){if(pomodoro_.running())pomodoro_.pause(now());else pomodoro_.start(now());arm();save();emit clocksChanged();}
void ProductivityModel::resetPomodoro(){phase_=0;pomodoro_.reset(phaseDuration());arm();save();emit clocksChanged();}
void ProductivityModel::advance(bool finished){
    if(phase_==0){if(finished)++rounds_;phase_=(finished && rounds_%4==0)?2:1;}else phase_=0;
    pomodoro_.reset(phaseDuration());
}
void ProductivityModel::skipPomodoro(){advance(false);arm();save();emit clocksChanged();}
void ProductivityModel::arm(){
    if(timer_.running() || stopwatch_.running() || pomodoro_.running())ticker_->start();else ticker_->stop();
}
void ProductivityModel::update(){
    bool changed=false;
    if(timer_.expired(now())){timer_.pause(now());changed=true;emit notice("Your timer is complete. Take a moment to stretch.");}
    if(pomodoro_.expired(now())){const bool focused=phase_==0;advance(true);changed=true;emit notice(focused?"A little progress. Your break is ready when you are.":"Welcome back. Your next focus session is ready.");}
    const qint64 stamp=now()/1000;
    if(stamp!=lastDisplay_ || changed){lastDisplay_=stamp;emit clocksChanged();}
    if(changed || now()-lastSave_>=30000){save();lastSave_=now();}
    arm();
}
void ProductivityModel::save(){
    if(!storageReadable_){emit notice(storageError_);return;}
    QJsonArray entries;for(const auto& item:items_)entries.append(QJsonObject{{"id",item.id},{"text",item.text},{"done",item.done},{"goal",item.goal}});
    QJsonObject root{{"version",1},{"items",entries},{"theme",theme_},{"timerMinutes",timerMinutes_},
        {"focusMinutes",focusMinutes_},{"shortMinutes",shortMinutes_},{"longMinutes",longMinutes_},{"phase",phase_},{"rounds",rounds_},
        {"timerRemaining",double(timer_.value(now()))},{"stopwatchElapsed",double(stopwatch_.value(now()))},{"pomodoroRemaining",double(pomodoro_.value(now()))}};
    QDir().mkpath(QFileInfo(path_).absolutePath());QSaveFile file(path_);
    if(!file.open(QIODevice::WriteOnly) || file.write(QJsonDocument(root).toJson())<0 || !file.commit()) {
        storageError_="Could not save your day: "+file.errorString();emit notice(storageError_);
    }else storageError_.clear();
}
void ProductivityModel::load(){
    QFile file(path_);if(!file.exists())return;
    if(!file.open(QIODevice::ReadOnly)){storageReadable_=false;storageError_="Could not open your saved tasks. Existing data has not been overwritten.";return;}
    QJsonParseError error;const auto doc=QJsonDocument::fromJson(file.readAll(),&error);
    if(error.error!=QJsonParseError::NoError || !doc.isObject()){
        storageReadable_=false;storageError_="Saved tasks could not be read. Original data has been preserved.";
        file.close();QFile::copy(path_,path_+".unreadable-backup");return;
    }
    const auto root=doc.object();
    for(const auto& entry:root["items"].toArray()){
        const auto obj=entry.toObject();const auto text=obj["text"].toString().trimmed().left(240);
        if(!text.isEmpty())items_.push_back({obj["id"].toString(QUuid::createUuid().toString()),text,obj["done"].toBool(),obj["goal"].toBool()});
    }
    const auto theme=root["theme"].toString();if(theme=="Rose linen" || theme=="Lavender dusk")theme_=theme;
    timerMinutes_=qBound(1,root["timerMinutes"].toInt(10),180);
    focusMinutes_=qBound(1,root["focusMinutes"].toInt(25),180);
    shortMinutes_=qBound(1,root["shortMinutes"].toInt(5),60);longMinutes_=qBound(1,root["longMinutes"].toInt(15),120);
    phase_=qBound(0,root["phase"].toInt(),2);rounds_=qMax(0,root["rounds"].toInt());
    timer_.reset(qint64(std::clamp(root["timerRemaining"].toDouble(timerMinutes_*60000),0.0,double(timerMinutes_*60000))));
    stopwatch_.reset(qint64(std::clamp(root["stopwatchElapsed"].toDouble(),0.0,double(365LL*24*3600000))));
    pomodoro_.reset(qint64(std::clamp(root["pomodoroRemaining"].toDouble(phaseDuration()),0.0,double(phaseDuration()))));
}
