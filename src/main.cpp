#include "PlayerWindow.h"
#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QMessageBox>
#include <QStandardPaths>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Live Wallpaper");
    QApplication::setOrganizationName("LiveWallpaperLearning");

    const auto data=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(data);
    QLockFile lock(data+"/application.lock");
    const bool testing=app.arguments().contains("--smoke-test") || app.arguments().contains("--import-test") || app.arguments().contains("--day-test") || app.arguments().contains("--setup-test");
    if(!testing && !lock.tryLock(0)) {
        QMessageBox::information(nullptr,"Live Wallpaper","Live Wallpaper is already running. Open it from the system tray.");
        return 0;
    }
    struct MutexOwner {HANDLE handle;~MutexOwner(){if(handle)CloseHandle(handle);}} mutex{testing?nullptr:CreateMutexW(nullptr,FALSE,L"Local\\HavenDesktopApp")};

    PlayerWindow window;
    window.show();
    // The event loop dispatches clicks, paint requests, and media notifications.
    return app.exec();
}
