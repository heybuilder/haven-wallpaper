#include "ProductivityPanel.h"
#include "ProductivityModel.h"
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
QLabel* label(const QString& text,const QString& name,QWidget* parent){auto* result=new QLabel(text,parent);result->setObjectName(name);result->setWordWrap(true);return result;}
QFrame* card(QWidget* parent){auto* frame=new QFrame(parent);frame->setObjectName("card");return frame;}
QSpinBox* minutes(QWidget* parent,int maximum,const QString& name){auto* value=new QSpinBox(parent);value->setRange(1,maximum);value->setSuffix(" min");value->setAccessibleName(name);return value;}
}

ProductivityPanel::ProductivityPanel(ProductivityModel* model,QWidget* parent):QWidget(parent),model_(model){
    auto* layout=new QHBoxLayout(this);layout->setContentsMargins(0,8,0,0);layout->setSpacing(18);
    auto* intentionsCard=card(this);auto* left=new QVBoxLayout(intentionsCard);left->setContentsMargins(18,18,18,18);
    left->addWidget(label("SMALL STEPS, AT YOUR PACE","eyebrow",this));
    progressText_=label("","muted",this);left->addWidget(progressText_);
    progress_=new QProgressBar(this);progress_->setTextVisible(false);progress_->setFixedHeight(6);left->addWidget(progress_);
    auto* lists=new QTabWidget(this);left->addWidget(lists,1);
    auto makeList=[this,lists](bool goal){
        auto* page=new QWidget(lists);auto* column=new QVBoxLayout(page);
        column->setContentsMargins(0,10,0,0);
        auto* inputRow=new QHBoxLayout;
        auto* input=new QLineEdit(page);input->setMaxLength(240);
        input->setPlaceholderText(goal?"An intention to grow toward...":"One small thing to do...");
        input->setAccessibleName(goal?"New goal":"New task");
        auto* add=new QPushButton("Add",page);inputRow->addWidget(input,1);inputRow->addWidget(add);column->addLayout(inputRow);
        auto* list=new QListWidget(page);list->setAccessibleName(goal?"Goals":"Tasks");column->addWidget(list,1);
        auto* remove=new QPushButton(goal?"Remove selected goal":"Remove selected task",page);column->addWidget(remove);
        auto submit=[this,input,goal]{if(!input->text().trimmed().isEmpty()){model_->add(input->text(),goal);input->clear();emit toolRequested(goal?1:0);}};
        connect(add,&QPushButton::clicked,this,submit);connect(input,&QLineEdit::returnPressed,this,submit);
        connect(list,&QListWidget::itemChanged,this,[this](QListWidgetItem* item){const auto id=item->data(Qt::UserRole).toString();const bool done=item->checkState()==Qt::Checked;QTimer::singleShot(0,this,[this,id,done]{model_->setDone(id,done);});});
        connect(remove,&QPushButton::clicked,this,[this,list]{if(auto* item=list->currentItem())model_->remove(item->data(Qt::UserRole).toString());});
        lists->addTab(page,goal?"Goals":"Tasks");return list;
    };
    tasks_=makeList(false);goals_=makeList(true);
    layout->addWidget(intentionsCard,1);
    auto* clockCard=card(this);auto* right=new QVBoxLayout(clockCard);right->setContentsMargins(18,18,18,18);
    right->addWidget(label("MAKE A LITTLE SPACE","eyebrow",this));
    auto* clocks=new QTabWidget(this);right->addWidget(clocks,1);
    auto* pomoPage=new QWidget;auto* pomo=new QVBoxLayout(pomoPage);
    phaseText_=label("","muted",pomoPage);phaseText_->setAlignment(Qt::AlignCenter);pomo->addWidget(phaseText_);
    pomodoroText_=label("25:00","clock",pomoPage);pomodoroText_->setAlignment(Qt::AlignCenter);pomo->addWidget(pomodoroText_);
    auto* pomoActions=new QHBoxLayout;pomodoroStart_=new QPushButton("Begin",pomoPage);pomodoroStart_->setProperty("primary",true);pomodoroStart_->setAccessibleName("Start or pause Pomodoro");
    auto* reset=new QPushButton("Reset",pomoPage);auto* skip=new QPushButton("Next phase",pomoPage);
    pomoActions->addWidget(pomodoroStart_);pomoActions->addWidget(reset);pomoActions->addWidget(skip);pomo->addLayout(pomoActions);
    auto* durations=new QHBoxLayout;
    auto duration=[&](const QString& text,int max){auto* column=new QVBoxLayout;column->addWidget(label(text,"muted",pomoPage));auto* spin=minutes(pomoPage,max,text);column->addWidget(spin);durations->addLayout(column);return spin;};
    focus_=duration("Focus",180);short_=duration("Short rest",60);long_=duration("Long rest",120);pomo->addLayout(durations);
    roundsText_=label("","muted",pomoPage);roundsText_->setAlignment(Qt::AlignCenter);pomo->addWidget(roundsText_);
    auto* guidance=label("A focus period, then a little rest. After four completed focuses, take a longer break. Start each phase when you feel ready.","muted",pomoPage);pomo->addWidget(guidance);pomo->addStretch();
    connect(pomodoroStart_,&QPushButton::clicked,this,[this]{model_->togglePomodoro();if(model_->pomodoroRunning())emit toolRequested(2);});
    connect(reset,&QPushButton::clicked,model_,&ProductivityModel::resetPomodoro);connect(skip,&QPushButton::clicked,model_,&ProductivityModel::skipPomodoro);
    auto setDurations=[this]{model_->setPomodoroMinutes(focus_->value(),short_->value(),long_->value());};
    connect(focus_,&QSpinBox::valueChanged,this,setDurations);connect(short_,&QSpinBox::valueChanged,this,setDurations);connect(long_,&QSpinBox::valueChanged,this,setDurations);
    clocks->addTab(pomoPage,"Pomodoro");
    auto* timerPage=new QWidget;auto* timerLayout=new QVBoxLayout(timerPage);
    timerLayout->addWidget(label("A little time for whatever you need.","muted",timerPage));
    timerText_=label("10:00","clock",timerPage);timerText_->setAlignment(Qt::AlignCenter);timerLayout->addWidget(timerText_);
    timerMinutes_=minutes(timerPage,180,"Countdown duration");timerLayout->addWidget(timerMinutes_);
    auto* timerActions=new QHBoxLayout;timerStart_=new QPushButton("Begin",timerPage);timerStart_->setProperty("primary",true);timerStart_->setAccessibleName("Start or pause countdown");auto* timerReset=new QPushButton("Reset",timerPage);
    timerActions->addWidget(timerStart_);timerActions->addWidget(timerReset);timerLayout->addLayout(timerActions);timerLayout->addStretch();clocks->addTab(timerPage,"Timer");
    connect(timerMinutes_,&QSpinBox::valueChanged,model_,&ProductivityModel::setTimerMinutes);
    connect(timerStart_,&QPushButton::clicked,this,[this]{model_->toggleTimer();if(model_->timerRunning())emit toolRequested(2);});connect(timerReset,&QPushButton::clicked,model_,&ProductivityModel::resetTimer);
    auto* stopwatchPage=new QWidget;auto* watch=new QVBoxLayout(stopwatchPage);watch->addWidget(label("Let time unfold. No finish line needed.","muted",stopwatchPage));
    stopwatchText_=label("00:00","clock",stopwatchPage);stopwatchText_->setAlignment(Qt::AlignCenter);watch->addWidget(stopwatchText_);
    auto* watchActions=new QHBoxLayout;stopwatchStart_=new QPushButton("Begin",stopwatchPage);stopwatchStart_->setProperty("primary",true);stopwatchStart_->setAccessibleName("Start or pause stopwatch");auto* watchReset=new QPushButton("Reset",stopwatchPage);
    watchActions->addWidget(stopwatchStart_);watchActions->addWidget(watchReset);watch->addLayout(watchActions);watch->addStretch();clocks->addTab(stopwatchPage,"Stopwatch");
    connect(stopwatchStart_,&QPushButton::clicked,this,[this]{model_->toggleStopwatch();if(model_->stopwatchRunning())emit toolRequested(2);});connect(watchReset,&QPushButton::clicked,model_,&ProductivityModel::resetStopwatch);
    layout->addWidget(clockCard,1);
    connect(model_,&ProductivityModel::intentionsChanged,this,&ProductivityPanel::refreshIntentions);
    connect(model_,&ProductivityModel::clocksChanged,this,&ProductivityPanel::refreshClocks);
    refreshIntentions();refreshClocks();
}
void ProductivityPanel::refreshIntentions(){
    auto selected=[](QListWidget* list){return list->currentItem()?list->currentItem()->data(Qt::UserRole).toString():QString();};
    const auto selectedTask=selected(tasks_),selectedGoal=selected(goals_);
    QSignalBlocker tasksBlock(tasks_),goalsBlock(goals_);tasks_->clear();goals_->clear();
    for(const auto& entry:model_->intentions()){
        auto* list=entry.goal?goals_:tasks_;auto* item=new QListWidgetItem(entry.text,list);
        item->setData(Qt::UserRole,entry.id);item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        item->setCheckState(entry.done?Qt::Checked:Qt::Unchecked);auto font=item->font();font.setStrikeOut(entry.done);item->setFont(font);
        if(entry.id==(entry.goal?selectedGoal:selectedTask))list->setCurrentItem(item);
    }
    const int total=model_->count(false)+model_->count(true),done=model_->count(false,true)+model_->count(true,true);
    progress_->setRange(0,qMax(1,total));progress_->setValue(done);
    progressText_->setText(total?QString("%1 of %2 little steps complete. Every bit counts.").arg(done).arg(total):"A fresh page. Start with one small intention.");
}
void ProductivityPanel::refreshClocks(){
    timerText_->setText(ProductivityModel::displayTime(model_->timerValue()));
    stopwatchText_->setText(ProductivityModel::displayTime((model_->stopwatchValue()/1000)*1000));
    pomodoroText_->setText(ProductivityModel::displayTime(model_->pomodoroValue()));phaseText_->setText(model_->phaseName());
    roundsText_->setText(QString("%1 focus sessions completed").arg(model_->rounds()));
    timerStart_->setText(model_->timerRunning()?"Pause":"Begin");stopwatchStart_->setText(model_->stopwatchRunning()?"Pause":"Begin");pomodoroStart_->setText(model_->pomodoroRunning()?"Pause":"Begin");
    QSignalBlocker a(timerMinutes_),b(focus_),c(short_),d(long_);
    timerMinutes_->setValue(model_->timerMinutes());focus_->setValue(model_->focusMinutes());short_->setValue(model_->shortMinutes());long_->setValue(model_->longMinutes());
    timerMinutes_->setEnabled(!model_->timerRunning());
    focus_->setEnabled(!model_->pomodoroRunning());short_->setEnabled(!model_->pomodoroRunning());long_->setEnabled(!model_->pomodoroRunning());
    timerMinutes_->setToolTip("Pause before changing duration. Changing it resets the countdown.");
    focus_->setToolTip("Pause before changing durations. Changes reset the current phase.");
}


ProductivityDock::ProductivityDock(ProductivityModel* model,QWidget* parent)
    :QDialog(nullptr,Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus),model_(model){
    Q_UNUSED(parent);
    setWindowTitle("Haven - active desktop tools");setObjectName("desktopTools");
    setAttribute(Qt::WA_TranslucentBackground);setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_QuitOnClose,false);
    setStyleSheet("QDialog#desktopTools{background:transparent;}"
        "QDialog#desktopTools QWidget{background:transparent;color:#fffdf6;font-family:'Segoe UI';}"
        "QDialog#desktopTools QFrame#overlayTile{background:rgba(25,35,31,100);border:1px solid rgba(255,255,255,28);border-radius:12px;}"
        "QDialog#desktopTools QLabel#overlayTime{font-size:38px;font-weight:300;}"
        "QDialog#desktopTools QLabel#overlayName{font-size:11px;color:#f2ecdc;}"
        "QDialog#desktopTools QCheckBox{font-size:13px;spacing:7px;padding:3px 0;}"
        "QDialog#desktopTools QCheckBox::indicator{width:13px;height:13px;border:1px solid rgba(255,255,255,160);border-radius:4px;background:rgba(255,255,255,18);}"
        "QDialog#desktopTools QCheckBox::indicator:checked{background:#bdcfb9;}");
    auto* stack=new QVBoxLayout(this);stack->setContentsMargins(4,4,4,4);stack->setSpacing(8);
    const QStringList names={"POMODORO","TIMER","STOPWATCH"};
    for(int i=0;i<3;++i){
        auto* tile=new QFrame(this);tile->setObjectName("overlayTile");clockTiles_[i]=tile;
        auto* column=new QVBoxLayout(tile);column->setContentsMargins(16,9,16,10);column->setSpacing(0);
        clockNames_[i]=new QLabel(names[i],tile);clockNames_[i]->setObjectName("overlayName");column->addWidget(clockNames_[i]);
        clockTimes_[i]=new QLabel("00:00",tile);clockTimes_[i]->setObjectName("overlayTime");column->addWidget(clockTimes_[i]);
        auto* shadow=new QGraphicsDropShadowEffect(clockTimes_[i]);shadow->setBlurRadius(8);shadow->setOffset(0,1);shadow->setColor(QColor(0,0,0,170));clockTimes_[i]->setGraphicsEffect(shadow);
        tile->hide();stack->addWidget(tile);
    }
    auto makeList=[this,stack](const QString& name,QVBoxLayout*& rows){
        auto* tile=new QFrame(this);tile->setObjectName("overlayTile");auto* column=new QVBoxLayout(tile);column->setContentsMargins(14,11,14,11);column->setSpacing(5);
        auto* title=new QLabel(name,tile);title->setObjectName("overlayName");column->addWidget(title);
        rows=new QVBoxLayout;rows->setContentsMargins(0,0,0,0);rows->setSpacing(1);column->addLayout(rows);tile->hide();stack->addWidget(tile);return tile;
    };
    taskTile_=makeList("TASKS",taskRows_);goalTile_=makeList("GOALS",goalRows_);
    QSettings settings;enabled_=settings.value("overlay/enabled",true).toBool();showTasks_=settings.value("overlay/tasks",false).toBool();showGoals_=settings.value("overlay/goals",false).toBool();
    corner_=settings.value("overlay/corner","Top right").toString();
    connect(model_,&ProductivityModel::clocksChanged,this,&ProductivityDock::refresh);
    connect(model_,&ProductivityModel::intentionsChanged,this,&ProductivityDock::refreshLists);
    connect(qApp,&QGuiApplication::screenRemoved,this,[this]{customPosition_=false;anchor();});
    setFixedWidth(256);refreshLists();
}
ProductivityDock::~ProductivityDock(){savePreferences();}
void ProductivityDock::savePreferences(){
    if(QCoreApplication::arguments().contains("--day-test"))return;
    QSettings settings;settings.setValue("overlay/enabled",enabled_);settings.setValue("overlay/tasks",showTasks_);settings.setValue("overlay/goals",showGoals_);settings.setValue("overlay/corner",corner_);settings.sync();
}
void ProductivityDock::setOverlayEnabled(bool value){enabled_=value;savePreferences();refresh();}
void ProductivityDock::setTasksVisible(bool value){showTasks_=value;savePreferences();refreshLists();}
void ProductivityDock::setGoalsVisible(bool value){showGoals_=value;savePreferences();refreshLists();}
void ProductivityDock::setCorner(const QString& value){
    if(!QStringList{"Top right","Bottom right","Top left","Bottom left"}.contains(value))return;
    corner_=value;customPosition_=false;savePreferences();anchor();
}
int ProductivityDock::activeClockCount() const {
    return int(model_->pomodoroRunning())+int(model_->timerRunning())+int(model_->stopwatchRunning());
}
void ProductivityDock::refreshLists(){
    auto rebuild=[this](bool goal,QWidget* tile,QVBoxLayout* rows){
        while(auto* entry=rows->takeAt(0)){if(entry->widget()){entry->widget()->hide();entry->widget()->deleteLater();}delete entry;}
        int shown=0,total=0;
        for(const auto& item:model_->intentions())if(item.goal==goal && !item.done){
            ++total;if(shown++>=4)continue;
            auto* check=new QCheckBox(tile);check->setText(check->fontMetrics().elidedText(item.text,Qt::ElideRight,205));check->setToolTip(item.text);check->setAccessibleName(item.text);rows->addWidget(check);
            connect(check,&QCheckBox::clicked,this,[this,id=item.id]{model_->setDone(id,true);});
        }
        if(total>4)rows->addWidget(new QLabel(QString("+ %1 more in Your day").arg(total-4),tile));
        tile->setProperty("openIntentions",total);
    };
    rebuild(false,taskTile_,taskRows_);rebuild(true,goalTile_,goalRows_);refresh();
}
void ProductivityDock::refresh(){
    const bool active[]={model_->pomodoroRunning(),model_->timerRunning(),model_->stopwatchRunning()};
    const qint64 values[]={model_->pomodoroValue(),model_->timerValue(),(model_->stopwatchValue()/1000)*1000};
    clockNames_[0]->setText("POMODORO / "+model_->phaseName());
    for(int i=0;i<3;++i){clockTiles_[i]->setVisible(active[i]);clockTimes_[i]->setText(ProductivityModel::displayTime(values[i]));}
    const bool tasks=showTasks_ && taskTile_->property("openIntentions").toInt()>0;
    const bool goals=showGoals_ && goalTile_->property("openIntentions").toInt()>0;
    taskTile_->setVisible(tasks);goalTile_->setVisible(goals);
    if(!enabled_ || (activeClockCount()==0 && !tasks && !goals)){hide();return;}
    layout()->activate();setFixedHeight(layout()->sizeHint().height());anchor();
    if(!isVisible())show();
}
void ProductivityDock::anchor(){
    auto* screen=QGuiApplication::screenAt(frameGeometry().center());if(!screen)screen=QGuiApplication::primaryScreen();if(!screen)return;
    const auto area=screen->availableGeometry();
    const int minX=area.left()+18,maxX=qMax(minX,area.right()-width()-18);
    const int minY=area.top()+18,maxY=qMax(minY,area.bottom()-height()-18);
    if(customPosition_)move(qBound(minX,x(),maxX),qBound(minY,y(),maxY));
    else move(corner_.endsWith("right")?maxX:minX,corner_.startsWith("Bottom")?maxY:minY);
}
void ProductivityDock::reveal(){enabled_=true;savePreferences();revealIfActive();}
void ProductivityDock::revealIfActive(){
    refresh();
    if(!isVisible())return;
    // Explorer or startup window ordering can hide a native tool window without
    // changing QWidget::isVisible(). Restore the native surface without focus.
    if(isMinimized())setWindowState(windowState() & ~Qt::WindowMinimized);
    show();raise();
    auto handle=reinterpret_cast<HWND>(winId());
    ShowWindow(handle,SW_SHOWNOACTIVATE);
    SetWindowPos(handle,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}
bool ProductivityDock::desktopVisible()const{return isVisible() && IsWindowVisible(reinterpret_cast<HWND>(effectiveWinId()));}
void ProductivityDock::mousePressEvent(QMouseEvent* event){
    if(event->button()==Qt::LeftButton){dragging_=true;dragOffset_=event->globalPosition().toPoint()-frameGeometry().topLeft();event->accept();}
}
void ProductivityDock::mouseMoveEvent(QMouseEvent* event){
    if(dragging_){customPosition_=true;move(event->globalPosition().toPoint()-dragOffset_);event->accept();}
}
void ProductivityDock::mouseReleaseEvent(QMouseEvent* event){dragging_=false;anchor();event->accept();}
void ProductivityDock::closeEvent(QCloseEvent* event){
    // Window/session teardown is not a request to change the saved overlay choice.
    if(property("quitting").toBool() || qApp->isSavingSession() || QCoreApplication::closingDown()){event->accept();return;}
    hide();event->ignore();
}
