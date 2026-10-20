#include "ImportSupport.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QTextStream>
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);int failed=0;
    auto check=[&](bool valid,const char* name){QTextStream(stdout)<<(valid?"PASS ":"FAIL ")<<name<<Qt::endl;if(!valid)++failed;};
    const QString canonical="https://www.youtube.com/watch?v=E0ux_as0N-s";
    check(ImportSupport::normalizeYoutube("https://youtu.be/E0ux_as0N-s?si=abc").toString()==canonical,"short links normalize without tracking parameters");
    check(ImportSupport::normalizeYoutube("https://www.youtube.com/shorts/E0ux_as0N-s").toString()==canonical,"Shorts links normalize");
    check(ImportSupport::normalizeYoutube("https://www.youtube.com/watch?v=E0ux_as0N-s&list=foo").toString()==canonical,"playlist parameter cannot trigger playlist downloads");
    check(ImportSupport::normalizeYoutube("https://youtube.com.attacker.test/watch?v=E0ux_as0N-s").isEmpty(),"non-YouTube hosts rejected");
    check(ImportSupport::normalizeYoutube("https://www.youtube.com/playlist?list=foo").isEmpty(),"non-video links rejected");
    QTemporaryDir dir(QDir::currentPath()+"/import-tests-XXXXXX");QDir().mkpath(dir.path()+"/cache");
    const auto video=dir.path()+QString::fromUtf8("/cache/caf\xc3\xa9.webm");QFile file(video);check(file.open(QIODevice::WriteOnly),"test video file opens");file.write("test");file.close();
    check(!ImportSupport::completedPath("WARNING: metadata warning\r\n"+video+"\r\n",dir.path()+"/cache").isEmpty(),"UTF-8 WebM output accepted despite metadata warnings");
    check(ImportSupport::completedPath(video,dir.path()+"/another-cache").isEmpty(),"paths outside import folder rejected");
    check(ImportSupport::completedPath("WARNING: metadata warning",dir.path()+"/cache").isEmpty(),"warnings alone cannot count as a completed import");
    check(ImportSupport::failureMessage("File is larger than max-filesize",0,false,false).contains("1 GB"),"size skips explain limit even with zero exit code");
    check(ImportSupport::failureMessage("",-1,true,true).contains("cancelled"),"cancellation is distinct from a crash");
    check(ImportSupport::failureMessage("",-1,true,false).contains("unexpectedly"),"crashes explain process result");
    return failed?1:0;
}
