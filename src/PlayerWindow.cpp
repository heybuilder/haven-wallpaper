#include "PlayerWindow.h"
#include "DesktopHost.h"
#include "ProductivityModel.h"
#include "ProductivityPanel.h"
#include "CalmTheme.h"
#include "ImportSupport.h"
#include "ImportJob.h"
#include <QApplication>
#include <QAudioOutput>
#include <QCloseEvent>
#include <QComboBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QDialog>
#include <QTextEdit>
#include <QDir>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMediaPlayer>
#include <QMenu>
#include <QProcess>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QSlider>
#include <QStandardPaths>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QVideoSink>
#include <memory>

PlayerWindow::PlayerWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Haven - Live Wallpaper & Your Day");
    resize(1160,850);setMinimumSize(940,760);
    const auto launchArgs=QCoreApplication::arguments();
    const int dayTest=launchArgs.indexOf("--day-test");
    const QString testStorage=(dayTest>=0 && launchArgs.size()>dayTest+1)?launchArgs.at(dayTest+1)+".json":QString();
    productivity_=new ProductivityModel(this,testStorage);
    qApp->setStyleSheet(calmStyle(productivity_->theme()));
    connect(productivity_,&ProductivityModel::themeChanged,this,[this]{qApp->setStyleSheet(calmStyle(productivity_->theme()));});
    auto* shell=new QWidget(this);auto* shellLayout=new QVBoxLayout(shell);
    shellLayout->setContentsMargins(24,20,24,16);shellLayout->setSpacing(14);
    auto* header=new QHBoxLayout;
    auto* brand=new QVBoxLayout;auto* eyebrow=new QLabel("H A V E N",shell);eyebrow->setObjectName("eyebrow");brand->addWidget(eyebrow);
    auto* welcome=new QLabel("A little room to breathe.",shell);welcome->setObjectName("heading");brand->addWidget(welcome);header->addLayout(brand,1);
    auto* theme=new QComboBox(shell);theme->addItems({"Sage morning","Rose linen","Lavender dusk"});theme->setCurrentText(productivity_->theme());theme->setAccessibleName("Ambience palette");header->addWidget(theme);
    connect(theme,&QComboBox::currentTextChanged,productivity_,&ProductivityModel::setTheme);
    auto* dayBarButton=new QPushButton("Desktop tools",shell);dayBarButton->setProperty("primary",true);header->addWidget(dayBarButton);
    shellLayout->addLayout(header);
    auto* pages=new QTabWidget(shell);shellLayout->addWidget(pages,1);
    auto* root=new QWidget(this);
    auto* layout=new QVBoxLayout(root);
    layout->setContentsMargins(0,10,0,0);layout->setSpacing(12);
    layout->addWidget(new QLabel("YOUR BACKDROP  /  A familiar place to settle in.",root));
    auto* imports=new QHBoxLayout;
    auto* local=new QPushButton("+ Local video",root);
    url_=new QLineEdit(root);url_->setPlaceholderText("Paste a YouTube video link");
    importButton_=new QPushButton("Import YouTube",root);
    auto* cancel=new QPushButton("Cancel import",root);
    auto* details=new QPushButton("Import details",root);
    imports->addWidget(local);imports->addWidget(url_,1);imports->addWidget(importButton_);imports->addWidget(cancel);imports->addWidget(details);
    connect(details,&QPushButton::clicked,this,[this]{
        QDialog dialog(this);dialog.setWindowTitle("YouTube import details");dialog.resize(800,480);
        auto* content=new QVBoxLayout(&dialog);auto* transcript=new QTextEdit(&dialog);transcript->setReadOnly(true);
        transcript->setPlainText(importOutput_.isEmpty()?"No import has been attempted in this session.":QString::fromUtf8(importOutput_));content->addWidget(transcript);
        auto* close=new QPushButton("Close",&dialog);content->addWidget(close);connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);dialog.exec();
    });
    layout->addLayout(imports);
    auto* content=new QHBoxLayout;
    library_=new QListWidget(root);library_->setMinimumWidth(260);
    library_->setAccessibleName("Wallpaper library");content->addWidget(library_,1);
    preview_=new QVideoWidget(root);preview_->setMinimumSize(420,260);
    content->addWidget(preview_,2);layout->addLayout(content,1);
    auto* actions=new QHBoxLayout;
    auto* previewButton=new QPushButton("Preview",root);
    auto* apply=new QPushButton("Set as wallpaper",root);
    apply->setProperty("primary",true);
    auto* stop=new QPushButton("Remove wallpaper",root);
    auto* remove=new QPushButton("Remove from library",root);
    monitors_=new QComboBox(root);monitors_->setAccessibleName("Target monitor");
    actions->addWidget(previewButton);actions->addWidget(monitors_);actions->addWidget(apply);
    actions->addWidget(stop);actions->addWidget(remove);layout->addLayout(actions);
    auto* playback=new QHBoxLayout;
    playButton_=new QPushButton("Play / pause",root);
    auto* volume=new QSlider(Qt::Horizontal,root);volume->setRange(0,100);volume->setValue(0);
    volume->setAccessibleName("Volume");volume->setMaximumWidth(180);
    playback->addWidget(playButton_);playback->addStretch();playback->addWidget(new QLabel("Volume",root));playback->addWidget(volume);
    layout->addLayout(playback);
    auto* cacheButton=new QPushButton("Open downloaded videos folder",root);
    layout->addWidget(cacheButton);
    connect(cacheButton,&QPushButton::clicked,this,[]{
        const auto cache=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/videos";
        QDir().mkpath(cache);QDesktopServices::openUrl(QUrl::fromLocalFile(cache));
    });
    remove->setToolTip("Removes this library entry. The original video stays on disk.");
    state_=new QLabel("Ready. Import or select a video. Audio starts muted.",root);
    state_->setWordWrap(true);state_->setObjectName("muted");layout->addWidget(state_);
    pages->addTab(root,"Your space");
    auto* dayPage=new QWidget(pages);auto* dayLayout=new QVBoxLayout(dayPage);dayLayout->setContentsMargins(0,12,0,0);
    auto* daySubtitle=new QLabel("Small steps. Soft surroundings. Your own pace.",dayPage);daySubtitle->setObjectName("muted");dayLayout->addWidget(daySubtitle);
    dayLayout->addWidget(new ProductivityPanel(productivity_,dayPage),1);pages->addTab(dayPage,"Your day");
    auto* gentleStatus=new QLabel("Make yourself at home. Nothing starts until you choose it.",shell);gentleStatus->setObjectName("muted");gentleStatus->setWordWrap(true);shellLayout->addWidget(gentleStatus);
    connect(productivity_,&ProductivityModel::notice,gentleStatus,&QLabel::setText);
    if(!productivity_->storageError().isEmpty())gentleStatus->setText(productivity_->storageError());
    dayBar_=new ProductivityDock(productivity_,this);
    auto* overlayOptions=new QHBoxLayout;
    auto* overlayEnabled=new QCheckBox("Show active tools on wallpaper",dayPage);overlayEnabled->setChecked(dayBar_->overlayEnabled());overlayOptions->addWidget(overlayEnabled);
    auto* overlayTasks=new QCheckBox("Tasks",dayPage);overlayTasks->setChecked(dayBar_->tasksVisible());overlayOptions->addWidget(overlayTasks);
    auto* overlayGoals=new QCheckBox("Goals",dayPage);overlayGoals->setChecked(dayBar_->goalsVisible());overlayOptions->addWidget(overlayGoals);
    auto* overlayCorner=new QComboBox(dayPage);overlayCorner->addItems({"Top right","Bottom right","Top left","Bottom left"});overlayCorner->setCurrentText(dayBar_->corner());overlayCorner->setAccessibleName("Desktop tool corner");overlayOptions->addWidget(overlayCorner);overlayOptions->addStretch();
    dayLayout->insertLayout(1,overlayOptions);
    connect(overlayEnabled,&QCheckBox::toggled,dayBar_,&ProductivityDock::setOverlayEnabled);
    connect(overlayTasks,&QCheckBox::toggled,dayBar_,&ProductivityDock::setTasksVisible);
    connect(overlayGoals,&QCheckBox::toggled,dayBar_,&ProductivityDock::setGoalsVisible);
    connect(overlayCorner,&QComboBox::currentTextChanged,dayBar_,&ProductivityDock::setCorner);
    connect(dayBarButton,&QPushButton::clicked,this,[this,pages]{pages->setCurrentIndex(1);showNormal();raise();});
    setCentralWidget(shell);
    player_=new QMediaPlayer(this);audio_=new QAudioOutput(this);audio_->setVolume(0);
    player_->setAudioOutput(audio_);player_->setVideoOutput(preview_);player_->setLoops(QMediaPlayer::Infinite);
    wallpaper_=new QVideoWidget;
    wallpaper_->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    wallpaper_->setAttribute(Qt::WA_NativeWindow);wallpaper_->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
    importer_=new QProcess(this);refreshMonitors();
    connect(importer_,&QProcess::started,this,[this]{
        if(importJob_ && !importJob_->attach(importer_->processId()))importOutput_+="[Haven] Process-tree cleanup could not be attached.\n";
    });
    connect(qApp,&QGuiApplication::screenAdded,this,[this]{refreshMonitors();});
    connect(qApp,&QGuiApplication::screenRemoved,this,[this]{removeWallpaper();refreshMonitors();});
    connect(local,&QPushButton::clicked,this,&PlayerWindow::openVideo);
    connect(previewButton,&QPushButton::clicked,this,[this]{playSelected(false);});
    connect(apply,&QPushButton::clicked,this,[this]{playSelected(true);});
    connect(stop,&QPushButton::clicked,this,&PlayerWindow::removeWallpaper);
    connect(playButton_,&QPushButton::clicked,this,&PlayerWindow::togglePlayback);
    connect(volume,&QSlider::valueChanged,this,[this](int v){audio_->setVolume(v/100.0f);});
    connect(importButton_,&QPushButton::clicked,this,&PlayerWindow::importYoutube);
    connect(cancel,&QPushButton::clicked,this,[this]{if(importer_->state()!=QProcess::NotRunning){importCancelled_=true;if(importJob_)importJob_->stop();importer_->kill();}});
    connect(remove,&QPushButton::clicked,this,[this]{delete library_->takeItem(library_->currentRow());saveLibrary();});
    connect(player_,&QMediaPlayer::errorOccurred,this,[this](QMediaPlayer::Error,const QString& message){state_->setText("Playback failed: "+message);});
    connect(importer_,&QProcess::readyReadStandardOutput,this,[this]{
        importOutput_+=importer_->readAllStandardOutput();
        if(importOutput_.size()>256000) importOutput_=importOutput_.right(256000);
        const auto lines=QString::fromUtf8(importOutput_).trimmed().split('\n');
        if(!lines.isEmpty()) state_->setText("Import: "+lines.last().left(220));
    });
    importer_->setProcessChannelMode(QProcess::MergedChannels);
    connect(importer_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status){
        importOutput_+=importer_->readAllStandardOutput();
        importButton_->setEnabled(true);
        importPath_=ImportSupport::completedPath(QString::fromUtf8(importOutput_),importFolder_);
        if(code==0 && status==QProcess::NormalExit && QFileInfo::exists(importPath_)) {
            addVideo(importPath_);state_->setText("YouTube import complete. Select Preview or Set as wallpaper. Some imports have no audio.");
        } else {state_->setText(ImportSupport::failureMessage(QString::fromUtf8(importOutput_),code,status==QProcess::CrashExit,importCancelled_));}
        importOutput_+="\n[Haven] exit="+QByteArray::number(code)+" crashed="+(status==QProcess::CrashExit?"yes":"no")+" saved="+importPath_.toUtf8()+"\n";
        importJob_.reset();
    });
    connect(importer_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError){importButton_->setEnabled(true);state_->setText("Importer error: "+importer_->errorString());});
    QSettings settings;
    const auto savedPaths=settings.value("library").toStringList();
    for(const auto& path:savedPaths) if(QFileInfo::exists(path)) addVideo(path);
    tray_=new QSystemTrayIcon(style()->standardIcon(QStyle::SP_ComputerIcon),this);
    auto* menu=new QMenu(this);
    menu->addAction("Open control panel",this,[this]{showNormal();raise();activateWindow();});
    menu->addAction("Desktop tools",this,[this,pages]{pages->setCurrentIndex(1);showNormal();raise();});
    menu->addAction("Play / pause",this,&PlayerWindow::togglePlayback);
    menu->addAction("Remove wallpaper",this,&PlayerWindow::removeWallpaper);
    menu->addSeparator();menu->addAction("Quit",this,&PlayerWindow::quitApplication);
    tray_->setContextMenu(menu);tray_->setToolTip("Haven - your desktop retreat");tray_->show();
    connect(productivity_,&ProductivityModel::notice,this,[this](const QString& message){tray_->showMessage("A moment for you",message,QSystemTrayIcon::Information,8000);});
    connect(tray_,&QSystemTrayIcon::activated,this,[this](QSystemTrayIcon::ActivationReason reason){if(reason==QSystemTrayIcon::DoubleClick){showNormal();raise();activateWindow();}});
    auto* health=new QTimer(this);
    connect(health,&QTimer::timeout,this,[this]{
        if(desktopActive_ && !DesktopHost::attached(wallpaper_->winId())) {
            removeWallpaper();state_->setText("Desktop changed. Select Set as wallpaper to reconnect.");
        }
    });health->start(3000);
    const auto args=QCoreApplication::arguments();
    const int smoke=args.indexOf("--smoke-test");
    if(smoke>=0 && args.size()>smoke+2) {
        const auto report=args.at(smoke+2);
        auto frames=std::make_shared<int>(0);
        auto log=[report](const QString& text){QFile file(report);if(file.open(QIODevice::Append))file.write((text+"\n").toUtf8());};
        connect(preview_->videoSink(),&QVideoSink::videoFrameChanged,this,[frames]{++*frames;});
        connect(wallpaper_->videoSink(),&QVideoSink::videoFrameChanged,this,[frames]{++*frames;});
        QTimer::singleShot(500,this,[this,args,smoke,log]{addVideo(args.at(smoke+1));playSelected(false);log("preview requested");});
        QTimer::singleShot(5000,this,[this,log,frames]{log(QString("preview frames=%1 state=%2 error=%3").arg(*frames).arg(player_->playbackState()).arg(player_->errorString()));*frames=0;playSelected(true);log(state_->text());});
        QTimer::singleShot(11000,this,[this,log,frames,args,smoke]{log(QString("wallpaper active=%1 attached=%2 frames=%3 state=%4 error=%5").arg(desktopActive_).arg(DesktopHost::attached(wallpaper_->winId())).arg(*frames).arg(player_->playbackState()).arg(player_->errorString()));removeWallpaper();log(QString("removed=%1").arg(!desktopActive_ && !DesktopHost::attached(wallpaper_->winId())));for(int i=library_->count()-1;i>=0;--i)if(library_->item(i)->data(Qt::UserRole).toString()==args.at(smoke+1))delete library_->takeItem(i);saveLibrary();quitApplication();});
    }
    const int importTest=args.indexOf("--import-test");
    if(importTest>=0 && args.size()>importTest+2) {
        const auto report=args.at(importTest+2);
        auto complete=[this,report]{QFile file(report);if(file.open(QIODevice::WriteOnly))file.write(state_->text().toUtf8());quitApplication();};
        connect(importer_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[complete](int,QProcess::ExitStatus){complete();});
        QTimer::singleShot(500,this,[this,args,importTest]{url_->setText(args.at(importTest+1));importYoutube();});
        QTimer::singleShot(300000,this,[this,complete]{if(importer_->state()!=QProcess::NotRunning){importCancelled_=true;if(importJob_)importJob_->stop();importer_->kill();state_->setText("Import test timed out. "+QString::fromUtf8(importOutput_.right(1200)));}complete();});
    }
    if(!testStorage.isEmpty()) {
        auto report=std::make_shared<QStringList>();
        auto check=[report](bool passed,const QString& name){*report<<(passed?"PASS ":"FAIL ")+name;};
        QTimer::singleShot(300,this,[this,pages,check]{
            pages->setCurrentIndex(1);productivity_->add("A small task",false);productivity_->add("A meaningful goal",true);
            productivity_->setTheme("Lavender dusk");dayBar_->setTasksVisible(true);dayBar_->setGoalsVisible(true);dayBar_->reveal();
            productivity_->toggleStopwatch();productivity_->togglePomodoro();productivity_->toggleTimer();
            check(productivity_->stopwatchRunning() && productivity_->pomodoroRunning() && productivity_->timerRunning(),"all three clocks run independently");
        });
        QTimer::singleShot(1800,this,[this,check]{
            check(productivity_->stopwatchValue()>1000,"stopwatch advances in the event loop");
            int taskViews=0,goalViews=0;
            for(auto* list:findChildren<QListWidget*>()) {
                if(list->accessibleName()=="Tasks" && list->count()>0)++taskViews;
                if(list->accessibleName()=="Goals" && list->count()>0)++goalViews;
            }
            check(taskViews==1 && goalViews==1,"full editors remain only in the main app");
            check(dayBar_->activeClockCount()==3 && dayBar_->width()==256 && dayBar_->isVisible(),"only active readouts appear in a compact overlay");
            check(dayBar_->windowFlags().testFlag(Qt::FramelessWindowHint) && dayBar_->testAttribute(Qt::WA_TranslucentBackground),"overlay is frameless and supports per-pixel transparency");
            productivity_->toggleStopwatch();productivity_->togglePomodoro();productivity_->toggleTimer();
            dayBar_->setTasksVisible(false);dayBar_->setGoalsVisible(false);
            check(!dayBar_->isVisible(),"overlay disappears when no tools are active");
        });
        QTimer::singleShot(2800,this,[this,check,report,launchArgs,dayTest]{
            productivity_->toggleTimer();check(dayBar_->isVisible() && dayBar_->activeClockCount()==1,"starting a clock automatically shows its readout");
            const auto snapshot=dayBar_->grab();snapshot.save(launchArgs.at(dayTest+1)+".png");
            const auto pixels=snapshot.toImage();
            check(pixels.pixelColor(0,0).alpha()==0 && pixels.pixelColor(12,28).alpha()>0 && pixels.pixelColor(12,28).alpha()<255,"overlay background renders with real transparency");
            dayBar_->setOverlayEnabled(false);check(!dayBar_->isVisible() && productivity_->timerRunning(),"hiding overlays does not stop timing");
            productivity_->toggleTimer();
            check(productivity_->storageError().isEmpty(),"productivity data saves successfully");
            QFile file(launchArgs.at(dayTest+1));if(file.open(QIODevice::WriteOnly))file.write(report->join('\n').toUtf8());
            quitApplication();
        });
    }
}
PlayerWindow::~PlayerWindow(){
    delete dayBar_;
    if(importJob_)importJob_->stop();
    if(importer_->state()!=QProcess::NotRunning){importer_->kill();importer_->waitForFinished(3000);}
    player_->stop();player_->setVideoOutput(nullptr);
    if(desktopActive_) DesktopHost::detach(wallpaper_->winId());
    delete wallpaper_;
}
void PlayerWindow::closeEvent(QCloseEvent* event){
    if(property("quitting").toBool()){event->accept();return;}
    if((desktopActive_ || dayBar_->isVisible() || productivity_->timerRunning() || productivity_->stopwatchRunning() || productivity_->pomodoroRunning()) && QSystemTrayIcon::isSystemTrayAvailable()) {
        hide();event->ignore();tray_->showMessage("Haven","Your space is still here. Use the tray menu to open or quit.");
    } else {event->accept();}
}
void PlayerWindow::quitApplication(){
    setProperty("quitting",true);dayBar_->setProperty("quitting",true);qApp->quit();
}
void PlayerWindow::refreshMonitors(){
    monitors_->clear();int index=1;
    for(auto* screen:QGuiApplication::screens()) monitors_->addItem(QString("Monitor %1 - %2").arg(index++).arg(screen->name()),screen->name());
}
void PlayerWindow::saveLibrary(){
    QStringList paths;for(int i=0;i<library_->count();++i) paths<<library_->item(i)->data(Qt::UserRole).toString();
    QSettings().setValue("library",paths);
}
void PlayerWindow::addVideo(const QString& path){
    for(int i=0;i<library_->count();++i) if(library_->item(i)->data(Qt::UserRole).toString()==path){library_->setCurrentRow(i);return;}
    auto* item=new QListWidgetItem(QFileInfo(path).completeBaseName(),library_);
    item->setData(Qt::UserRole,path);item->setToolTip(path);library_->setCurrentItem(item);saveLibrary();
}
void PlayerWindow::openVideo(){
    const auto paths=QFileDialog::getOpenFileNames(this,"Add local videos",{},"Videos (*.mp4 *.mkv *.webm *.mov *.avi);;All files (*)");
    for(const auto& path:paths) addVideo(path);
    if(!paths.isEmpty()) state_->setText("Added to library. Select Preview or Set as wallpaper.");
}
void PlayerWindow::playSelected(bool wallpaper){
    auto* item=library_->currentItem();
    if(!item){state_->setText("Select a video from your library first.");return;}
    auto path=item->data(Qt::UserRole).toString();
    if(!QFileInfo::exists(path)){state_->setText("Video file was moved or deleted. Add it again.");return;}
    player_->stop();if(desktopActive_) removeWallpaper();
    if(wallpaper){
        QString error;
        wallpaper_->setGeometry(-32000,-32000,640,480);wallpaper_->show();
        if(!DesktopHost::attach(wallpaper_->winId(),monitors_->currentData().toString(),error)){wallpaper_->hide();state_->setText(error);return;}
        desktopActive_=true;player_->setVideoOutput(wallpaper_);
    } else player_->setVideoOutput(preview_);
    player_->setSource(QUrl::fromLocalFile(path));player_->play();
    state_->setText((wallpaper?"Wallpaper active: ":"Previewing: ")+item->text());
}
void PlayerWindow::removeWallpaper(){
    player_->stop();
    if(desktopActive_){DesktopHost::detach(wallpaper_->winId());desktopActive_=false;}
    player_->setVideoOutput(preview_);state_->setText("Stopped. Your normal desktop is visible.");
}
void PlayerWindow::togglePlayback(){
    if(player_->source().isEmpty()) return;
    if(player_->playbackState()==QMediaPlayer::PlayingState) player_->pause();else player_->play();
}
void PlayerWindow::importYoutube(){
    if(importer_->state()!=QProcess::NotRunning) return;
    const auto url=ImportSupport::normalizeYoutube(url_->text());
    if(url.isEmpty()) {
        state_->setText("Enter a valid HTTPS YouTube video link.");return;
    }
    const auto tools=QCoreApplication::applicationDirPath()+"/../tools/";
    const auto exe=tools+"yt-dlp.exe";
    if(!QFileInfo::exists(exe) || !QFileInfo::exists(tools+"deno.exe")) {
        state_->setText("YouTube tools are missing. Install yt-dlp.exe and deno.exe in tools beside preview.");return;
    }
    const auto cache=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/videos";
    if(!QDir().mkpath(cache)){state_->setText("Cannot create the video cache folder.");return;}
    const auto importFolder=cache+"/"+QUuid::createUuid().toString(QUuid::WithoutBraces);
    if(!QDir().mkpath(importFolder)){state_->setText("Cannot create import folder.");return;}
    importPath_.clear();
    importFolder_=importFolder;importCancelled_=false;
    importJob_=std::make_unique<ImportJob>();
    importOutput_.clear();importButton_->setEnabled(false);state_->setText("Importing up to 1080p (1 GB limit). This may take a few minutes...");
    importer_->start(exe,{"--ignore-config","--no-playlist","--newline","--encoding","utf-8","--windows-filenames","--js-runtimes","deno:"+tools+"deno.exe",
                         "--max-filesize","1G","--socket-timeout","20","--retries","2","--no-simulate","--print","after_move:filepath",
                         "-f",ImportSupport::formatSelector(),"-o",importFolder+"/%(title).100B [%(id)s].%(ext)s","--",url.toString()});
}

