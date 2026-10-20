#pragma once
#include "FocusClock.h"
#include <QObject>
#include <QElapsedTimer>
#include <QVector>
#include <QString>
#include <functional>
class QTimer;

struct Intention {QString id;QString text;bool done=false;bool goal=false;};
class ProductivityModel final : public QObject {
    Q_OBJECT
public:
    explicit ProductivityModel(QObject* parent=nullptr, const QString& storage={},std::function<qint64()> clock={});
    ~ProductivityModel() override;
    const QVector<Intention>& intentions() const {return items_;}
    void add(const QString& text,bool goal);
    void setDone(const QString& id,bool done);
    void remove(const QString& id);
    int count(bool goal,bool completed=false) const;
    QString theme() const {return theme_;}
    void setTheme(const QString& theme);
    qint64 timerValue() const {return timer_.value(now());}
    qint64 stopwatchValue() const {return stopwatch_.value(now());}
    qint64 pomodoroValue() const {return pomodoro_.value(now());}
    bool timerRunning() const {return timer_.running();}
    bool stopwatchRunning() const {return stopwatch_.running();}
    bool pomodoroRunning() const {return pomodoro_.running();}
    int timerMinutes() const {return timerMinutes_;}
    int focusMinutes() const {return focusMinutes_;}
    int shortMinutes() const {return shortMinutes_;}
    int longMinutes() const {return longMinutes_;}
    int rounds() const {return rounds_;}
    int phase() const {return phase_;}
    QString phaseName() const;
    void setTimerMinutes(int minutes);
    void setPomodoroMinutes(int focus,int shortBreak,int longBreak);
    void toggleTimer();void resetTimer();
    void toggleStopwatch();void resetStopwatch();
    void togglePomodoro();void resetPomodoro();void skipPomodoro();
    QString storageError() const {return storageError_;}
    static QString displayTime(qint64 milliseconds);
    void refreshTime(){update();}
signals:
    void intentionsChanged();
    void clocksChanged();
    void themeChanged();
    void notice(const QString& message);
private:
    qint64 now() const {return clock_?clock_():elapsed_.elapsed();}
    void update();void arm();void save();void load();void advance(bool finished);
    qint64 phaseDuration() const;
    QElapsedTimer elapsed_;
    std::function<qint64()> clock_;
    QTimer* ticker_;
    FocusClock timer_{true,10*60000};
    FocusClock stopwatch_{false};
    FocusClock pomodoro_{true,25*60000};
    QVector<Intention> items_;
    QString theme_="Sage morning";
    QString path_;
    QString storageError_;
    bool storageReadable_=true;
    int timerMinutes_=10,focusMinutes_=25,shortMinutes_=5,longMinutes_=15;
    int phase_=0,rounds_=0;
    qint64 lastDisplay_=-1,lastSave_=0;
};
