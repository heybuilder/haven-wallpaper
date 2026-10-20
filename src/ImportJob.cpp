#include "ImportJob.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
ImportJob::ImportJob(){
    auto job=CreateJobObjectW(nullptr,nullptr);if(!job)return;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if(!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))){CloseHandle(job);return;}
    handle_=job;
}
ImportJob::~ImportJob(){if(handle_)CloseHandle(static_cast<HANDLE>(handle_));}
bool ImportJob::attach(qint64 id){
    if(!handle_)return false;
    auto process=OpenProcess(PROCESS_SET_QUOTA|PROCESS_TERMINATE,FALSE,static_cast<DWORD>(id));if(!process)return false;
    const bool result=AssignProcessToJobObject(static_cast<HANDLE>(handle_),process);CloseHandle(process);return result;
}
void ImportJob::stop(){if(handle_)TerminateJobObject(static_cast<HANDLE>(handle_),1);}
