#include "YouTubeSetup.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryDir>

namespace {
const QString urls[]={
    "https://github.com/yt-dlp/yt-dlp/releases/download/2026.08.19/yt-dlp.exe",
    "https://github.com/denoland/deno/releases/download/v2.9.7/deno-x86_64-pc-windows-msvc.zip"};
const QByteArray checksums[]={
    "66674953fe251b89f4d08c5f0e35e0728679bd67ab3d7d05c0562af101dd3e7a",
    "a0c3101b4158d1dfb7d6a78a7bf0f3de80c96bb423c152beec8beb22786f2238"};
const QString names[]={"yt-dlp.exe","deno.zip"};
QString localTools(){return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/youtube-tools";}
bool complete(const QString& path){return QFileInfo(path+"/yt-dlp.exe").isFile() && QFileInfo(path+"/deno.exe").isFile();}
QString literal(QString path){return "'"+path.replace("'","''")+"'";}
}
QString YouTubeSetup::toolsPath(){
    if(complete(localTools()))return localTools()+"/";
    const auto sibling=QCoreApplication::applicationDirPath()+"/../tools";
    return (complete(sibling)?sibling:localTools())+"/";
}
bool YouTubeSetup::ready(){return complete(toolsPath());}
YouTubeSetup::YouTubeSetup(QObject* parent,const QString& installRoot):QObject(parent),destination_(installRoot){
    network_=new QNetworkAccessManager(this);extractor_=new QProcess(this);
    connect(extractor_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status){
        if(!busy_)return;
        if(code!=0 || status!=QProcess::NormalExit){fail("Could not unpack the YouTube runtime. Please retry setup.");return;}
        install();
    });
    connect(extractor_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError){if(busy_)fail("Could not start Windows ZIP extraction: "+extractor_->errorString());});
}
YouTubeSetup::~YouTubeSetup(){
    busy_=false;if(reply_){reply_->disconnect(this);reply_->abort();}
    if(extractor_->state()!=QProcess::NotRunning){extractor_->kill();extractor_->waitForFinished(2000);}
}
void YouTubeSetup::start(){
    if(busy_)return;
    if(destination_.isEmpty() && ready()){emit finished(true,"YouTube imports are ready. Paste a video link to begin.");return;}
    const auto base=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if(!QDir().mkpath(base)){emit finished(false,"Cannot create the YouTube tools folder.");return;}
    staging_=std::make_unique<QTemporaryDir>(base+"/youtube-setup-XXXXXX");
    if(!staging_->isValid()){staging_.reset();emit finished(false,"Cannot create the temporary setup folder.");return;}
    busy_=true;step_=0;downloadNext();
}
void YouTubeSetup::downloadNext(){
    received_=0;ioError_.clear();
    file_=std::make_unique<QSaveFile>(staging_->path()+"/"+names[step_]);
    if(!file_->open(QIODevice::WriteOnly)){fail("Cannot write downloaded helper files.");return;}
    hash_=std::make_unique<QCryptographicHash>(QCryptographicHash::Sha256);
    QNetworkRequest request{QUrl(urls[step_])};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(30000);
    reply_=network_->get(request);
    emit progress(QString("Downloading %1 of 2 from the official project...").arg(step_+1));
    connect(reply_,&QNetworkReply::readyRead,this,[this]{
        const auto data=reply_->readAll();received_+=data.size();
        if(received_>160LL*1024*1024){ioError_="The helper download exceeded its size limit.";reply_->abort();return;}
        hash_->addData(data);
        if(file_->write(data)!=data.size()){ioError_="Could not save the helper download.";reply_->abort();}
    });
    connect(reply_,&QNetworkReply::downloadProgress,this,[this](qint64 downloaded,qint64 total){
        emit progress(QString("Setting up YouTube: file %1/2, %2 MB%3").arg(step_+1).arg(downloaded/(1024*1024)).arg(total>0?QString(" / %1 MB").arg(total/(1024*1024)):QString()));
    });
    connect(reply_,&QNetworkReply::finished,this,[this]{
        auto* response=reply_;reply_=nullptr;
        const auto error=response->error();const auto message=response->errorString();response->deleteLater();
        if(!busy_)return;
        if(error!=QNetworkReply::NoError || !ioError_.isEmpty()){fail(ioError_.isEmpty()?"Helper download failed: "+message+". Check your connection and retry.":ioError_);return;}
        if(hash_->result().toHex()!=checksums[step_]){fail("The helper checksum did not match. Nothing was installed. Please retry later.");return;}
        if(!file_->commit()){fail("Could not save the verified helper download.");return;}
        file_.reset();hash_.reset();
        if(++step_<2)downloadNext();else extract();
    });
}
void YouTubeSetup::extract(){
    emit progress("Downloads verified. Unpacking the YouTube runtime...");
    const auto powershell=qEnvironmentVariable("SystemRoot")+"/System32/WindowsPowerShell/v1.0/powershell.exe";
    const auto script="$ErrorActionPreference='Stop'; Expand-Archive -LiteralPath "+literal(staging_->path()+"/deno.zip")+" -DestinationPath "+literal(staging_->path()+"/runtime")+" -Force";
    QByteArray encoded;
    for(const auto ch:script){const auto code=ch.unicode();encoded.append(char(code&255));encoded.append(char(code>>8));}
    extractor_->start(powershell,{"-NoProfile","-NonInteractive","-EncodedCommand",QString::fromLatin1(encoded.toBase64())});
}
void YouTubeSetup::install(){
    const auto destination=destination_.isEmpty()?localTools():destination_;
    if(!QDir().mkpath(destination)){fail("Cannot create the YouTube tools folder.");return;}
    for(const auto& pair:{qMakePair(staging_->path()+"/yt-dlp.exe",destination+"/yt-dlp.exe"),qMakePair(staging_->path()+"/runtime/deno.exe",destination+"/deno.exe")}){
        QFile input(pair.first);QSaveFile output(pair.second);
        if(!input.open(QIODevice::ReadOnly) || !output.open(QIODevice::WriteOnly)){fail("Cannot install YouTube helpers. Close other Haven copies and retry.");return;}
        while(!input.atEnd()){const auto data=input.read(1024*1024);if(data.isEmpty() || output.write(data)!=data.size()){fail("Could not copy the verified helper files.");return;}}
        if(!output.commit()){fail("Could not finish installing YouTube helpers.");return;}
    }
    busy_=false;staging_.reset();emit finished(true,"YouTube imports are ready. Paste a video link to begin.");
}
void YouTubeSetup::fail(const QString& message){
    busy_=false;file_.reset();hash_.reset();staging_.reset();emit finished(false,message);
}
void YouTubeSetup::cancel(){
    if(!busy_)return;
    busy_=false;
    if(reply_){auto* response=reply_;reply_=nullptr;response->disconnect(this);response->abort();response->deleteLater();}
    if(extractor_->state()!=QProcess::NotRunning){extractor_->kill();extractor_->waitForFinished(2000);}
    file_.reset();hash_.reset();staging_.reset();emit finished(false,"YouTube setup cancelled. Your wallpapers and tasks are unchanged.");
}
