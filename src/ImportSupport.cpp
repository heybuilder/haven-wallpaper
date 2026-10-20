#include "ImportSupport.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrlQuery>
QUrl ImportSupport::normalizeYoutube(const QString& input){
    const auto url=QUrl::fromUserInput(input.trimmed());const auto host=url.host().toLower();
    if(url.scheme()!="https" || !(host=="youtu.be" || host=="youtube.com" || host.endsWith(".youtube.com")))return {};
    QString id;const auto parts=url.path().split('/',Qt::SkipEmptyParts);
    if(host=="youtu.be" && parts.size()==1)id=parts.first();
    else if(url.path()=="/watch")id=QUrlQuery(url).queryItemValue("v");
    else if(parts.size()==2 && (parts[0]=="shorts" || parts[0]=="live" || parts[0]=="embed"))id=parts[1];
    if(!QRegularExpression("^[A-Za-z0-9_-]{11}$").match(id).hasMatch())return {};
    return QUrl("https://www.youtube.com/watch?v="+id);
}
QString ImportSupport::formatSelector(){
    // Avoid selecting multi-gigabyte 4K formats. No merging dependency is needed.
    const QString limit="[height<=1080][filesize<=?1G][filesize_approx<=?1G]";
    return "best[ext=mp4][vcodec^=avc1]"+limit+"/bestvideo[ext=mp4][vcodec^=avc1]"+limit;
}
QString ImportSupport::completedPath(const QString& output,const QString& folder){
    const auto root=QDir(folder).canonicalPath()+"/";
    for(const auto& line:output.split('\n')){
        QFileInfo info(line.trimmed());
        if(!info.isFile())continue;
        const auto suffix=info.suffix().toLower();
        if((suffix=="mp4" || suffix=="webm" || suffix=="mkv") && info.canonicalFilePath().startsWith(root,Qt::CaseInsensitive))return info.canonicalFilePath();
    }
    return {};
}
QString ImportSupport::failureMessage(const QString& output,int code,bool crashed,bool cancelled){
    if(cancelled)return "Import cancelled. Your wallpaper is unchanged.";
    const auto lower=output.toLower();
    if(lower.contains("max-filesize") || lower.contains("larger than max") || lower.contains("file is larger"))return "This video exceeds the 1 GB import limit. Try a shorter video.";
    if(lower.contains("sign in") || lower.contains("private video") || lower.contains("age-restricted") || lower.contains("members-only"))return "YouTube requires account access for this video. Try a public, unrestricted video.";
    if(lower.contains("not available") || lower.contains("unavailable") || lower.contains("removed"))return "This video or a compatible format is unavailable. Try another public video; see Import details.";
    if(lower.contains("timed out") || lower.contains("unable to download") || lower.contains("name resolution") || lower.contains("http error"))return "YouTube could not be reached reliably. Check your connection and retry; see Import details.";
    if(crashed)return QString("The YouTube downloader stopped unexpectedly (code %1). See Import details.").arg(code);
    if(code==0)return "YouTube finished without saving a playable video. See Import details for the size or format reason.";
    return QString("YouTube import failed (code %1). See Import details for the full error.").arg(code);
}
