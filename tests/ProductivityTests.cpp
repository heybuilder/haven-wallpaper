#include "FocusClock.h"
#include "ProductivityModel.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>

int main(int argc,char** argv){
    QCoreApplication app(argc,argv);int failures=0;
    auto check=[&](bool condition,const char* name){QTextStream(stdout)<<(condition?"PASS ":"FAIL ")<<name<<Qt::endl;if(!condition)++failures;};
    FocusClock countdown(true,10000);countdown.start(1000);
    check(countdown.value(4500)==6500,"countdown uses elapsed time");
    countdown.pause(4500);check(countdown.value(9000)==6500,"paused countdown stays still");
    countdown.start(10000);check(countdown.value(12000)==4500,"countdown resumes without losing prior time");
    check(countdown.expired(20000) && countdown.value(20000)==0,"countdown clamps delayed ticks at zero");
    FocusClock watch(false);watch.start(0);watch.pause(3500);watch.start(8000);
    check(watch.value(10000)==5500,"stopwatch excludes paused intervals");
    watch.reset(0);check(!watch.running() && watch.value(20000)==0,"stopwatch reset stops and clears");
    QTemporaryDir temporary(QDir::currentPath()+"/productivity-test-XXXXXX");check(temporary.isValid(),"temporary persistence directory created");
    const auto path=temporary.path()+"/state.json";qint64 now=0;
    {
        ProductivityModel model(nullptr,path,[&]{return now;});int notifications=0;
        QObject::connect(&model,&ProductivityModel::notice,&model,[&](const QString&){++notifications;});
        model.add("  Read a chapter  ",false);model.add("Finish my project",true);model.add("   ",false);
        check(model.count(false)==1 && model.count(true)==1,"tasks and goals are separate; blank input ignored");
        model.setDone(model.intentions().front().id,true);
        check(model.count(false,true)==1,"task completion stored by ID");
        model.setTheme("Rose linen");model.setTheme("unknown");check(model.theme()=="Rose linen","theme validation");
        model.setTimerMinutes(1);model.toggleTimer();now+=61000;model.refreshTime();
        check(!model.timerRunning() && model.timerValue()==0 && notifications==1,"timer completes once despite delayed update");
        model.refreshTime();check(notifications==1,"timer completion does not repeat");
        model.setPomodoroMinutes(1,1,2);
        for(int round=1;round<=4;++round){
            model.togglePomodoro();now+=61000;model.refreshTime();
            check(model.rounds()==round && !model.pomodoroRunning(),"completed focus counts once and waits for user");
            check(model.phase()==(round==4?2:1),"fourth focus chooses a long rest");
            model.togglePomodoro();now+=(round==4?121000:61000);model.refreshTime();
            check(model.phase()==0 && !model.pomodoroRunning(),"finished rest returns to a paused focus");
        }
        model.skipPomodoro();check(model.rounds()==4 && model.phase()==1,"skipping never earns a completed focus");
        model.toggleStopwatch();now+=15000;model.refreshTime();
        model.setTimerMinutes(12);model.toggleTimer();now+=5000;
    }
    {
        ProductivityModel restored(nullptr,path,[&]{return now;});
        check(restored.count(false)==1 && restored.count(true)==1 && restored.count(false,true)==1,"tasks and goals survive restart");
        check(restored.theme()=="Rose linen" && restored.rounds()==4,"theme and Pomodoro progress survive restart");
        check(!restored.timerRunning() && !restored.stopwatchRunning() && !restored.pomodoroRunning(),"restored clocks wait for an explicit start");
        check(restored.timerValue()==715000 && restored.stopwatchValue()==20000,"clock snapshots preserve time on exit");
        restored.remove(restored.intentions().front().id);check(restored.count(false)==0,"remove selected intention only");
    }
    check(ProductivityModel::displayTime(25000)=="00:25" && ProductivityModel::displayTime(3600000)=="01:00:00","clock formatting handles hours");
    return failures?1:0;
}
