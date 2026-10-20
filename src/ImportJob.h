#pragma once
#include <QtGlobal>
class ImportJob {
public:
    ImportJob();~ImportJob();
    ImportJob(const ImportJob&)=delete;ImportJob& operator=(const ImportJob&)=delete;
    bool attach(qint64 processId);
    void stop();
private:
    void* handle_=nullptr;
};
