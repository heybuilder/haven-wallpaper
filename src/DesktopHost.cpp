#include "DesktopHost.h"
#include <QStringList>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
BOOL CALLBACK findWorker(HWND window, LPARAM data) {
    if (FindWindowExW(window, nullptr, L"SHELLDLL_DefView", nullptr)) {
        auto worker = FindWindowExW(nullptr, window, L"WorkerW", nullptr);
        if (worker) { *reinterpret_cast<HWND*>(data) = worker; return FALSE; }
    }
    return TRUE;
}
struct MonitorSearch { QString name; RECT rect{}; bool found=false; };
BOOL CALLBACK describeWindow(HWND window, LPARAM data) {
    wchar_t name[256]{};GetClassNameW(window,name,256);
    auto& names=*reinterpret_cast<QStringList*>(data);
    names<<QString::fromWCharArray(name);
    return TRUE;
}
BOOL CALLBACK collectMonitor(HMONITOR monitor, HDC, LPRECT, LPARAM data) {
    MONITORINFOEXW info{}; info.cbSize=sizeof(info);
    auto& search=*reinterpret_cast<MonitorSearch*>(data);
    if (GetMonitorInfoW(monitor, &info) && QString::fromWCharArray(info.szDevice)==search.name) {
        search.rect=info.rcMonitor; search.found=true; return FALSE;
    }
    return TRUE;
}
}
bool DesktopHost::attach(WId id, const QString& monitor, QString& error) {
    HWND progman = FindWindowW(L"Progman", nullptr);
    if (!progman) { error="Windows Explorer desktop was not found."; return false; }
    DWORD_PTR result=0;
    // Explorer's undocumented protocol is isolated here from UI and playback.
    SendMessageTimeoutW(progman,0x052C,0xD,0,SMTO_ABORTIFHUNG,1000,&result);
    SendMessageTimeoutW(progman,0x052C,0xD,1,SMTO_ABORTIFHUNG,1000,&result);
    HWND worker=nullptr;
    EnumWindows(findWorker,reinterpret_cast<LPARAM>(&worker));
    // Windows 11 can create WorkerW inside Progman rather than at the top level.
    if(!worker) worker=FindWindowExW(progman,nullptr,L"WorkerW",nullptr);
    if(!worker) {
        SendMessageTimeoutW(progman,0x052C,0,0,SMTO_ABORTIFHUNG,1000,&result);
        EnumWindows(findWorker,reinterpret_cast<LPARAM>(&worker));
        if(!worker) worker=FindWindowExW(progman,nullptr,L"WorkerW",nullptr);
    }
    if (!worker) {
        QStringList names;EnumChildWindows(progman,describeWindow,reinterpret_cast<LPARAM>(&names));
        error="This Windows desktop layout is unsupported. Wallpaper was not applied. Desktop children: "+names.join(", ");return false;
    }
    MonitorSearch search; search.name=monitor;
    EnumDisplayMonitors(nullptr,nullptr,collectMonitor,reinterpret_cast<LPARAM>(&search));
    if (!search.found) { error="The selected monitor is no longer available."; return false; }
    const auto rect=search.rect;
    HWND window=reinterpret_cast<HWND>(id);
    auto style=GetWindowLongPtrW(window,GWL_STYLE);
    SetWindowLongPtrW(window,GWL_STYLE,(style & ~WS_POPUP) | WS_CHILD);
    SetLastError(0);
    auto previous=SetParent(window,worker);
    if (!previous && GetLastError()!=0) {
        SetWindowLongPtrW(window,GWL_STYLE,style); error="Windows rejected desktop attachment."; return false;
    }
    POINT origin{rect.left,rect.top}; ScreenToClient(worker,&origin);
    if(!SetWindowPos(window,HWND_BOTTOM,origin.x,origin.y,rect.right-rect.left,rect.bottom-rect.top,
                 SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW)) {
        detach(id);error="Could not position the wallpaper.";return false;
    }
    return true;
}
void DesktopHost::detach(WId id) {
    auto window=reinterpret_cast<HWND>(id); ShowWindow(window,SW_HIDE); SetParent(window,nullptr);
    auto style=GetWindowLongPtrW(window,GWL_STYLE);
    SetWindowLongPtrW(window,GWL_STYLE,(style & ~WS_CHILD) | WS_POPUP);
}
bool DesktopHost::attached(WId id) {
    HWND parent=GetParent(reinterpret_cast<HWND>(id)); return parent && IsWindow(parent);
}
