#pragma once
#include <QtGlobal>
#include <algorithm>

// Timestamps are supplied by a monotonic clock, so UI delays do not lose time.
class FocusClock {
public:
    explicit FocusClock(bool countdown=true, qint64 initial=0)
        : countdown_(countdown), value_(initial) {}
    qint64 value(qint64 now) const {
        const auto elapsed=running_ ? std::max<qint64>(0,now-anchor_) : 0;
        return countdown_ ? std::max<qint64>(0,value_-elapsed) : value_+elapsed;
    }
    bool running() const {return running_;}
    void start(qint64 now) {if(!running_ && (!countdown_ || value_>0)){anchor_=now;running_=true;}}
    void pause(qint64 now) {value_=value(now);running_=false;}
    void reset(qint64 value) {value_=std::max<qint64>(0,value);running_=false;}
    bool expired(qint64 now) const {return countdown_ && running_ && value(now)==0;}
private:
    bool countdown_;
    qint64 value_;
    qint64 anchor_=0;
    bool running_=false;
};
