#include "PlayerWindow.h"
#include "MediaController.h"
#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSettings>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Live Wallpaper");
    QApplication::setOrganizationName("LiveWallpaperLearning");

    if(app.arguments().contains("--media-worker")) {
        if(QSettings().value("compatibilityPlayback",false).toBool() || qEnvironmentVariableIsSet("HAVEN_SAFE_PLAYBACK")) {
            qputenv("QT_FFMPEG_DECODING_HW_DEVICE_TYPES",",");qputenv("QT_DISABLE_HW_TEXTURES_CONVERSION","1");
        }
        return runMediaWorker(app);
    }
    const auto data=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(data);
    QLockFile lock(data+"/application.lock");
    const bool testing=app.arguments().contains("--restore-tools-test") || app.arguments().contains("--layout-test") || app.arguments().contains("--smoke-test") || app.arguments().contains("--import-test") || app.arguments().contains("--day-test") || app.arguments().contains("--setup-test");
    if(testing) {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,app.arguments().last()+".settings");
    }
    if(!testing && !lock.tryLock(0)) {
        QMessageBox::information(nullptr,"Live Wallpaper","Live Wallpaper is already running. Open it from the system tray.");
        return 0;
    }
    struct MutexOwner {HANDLE handle;~MutexOwner(){if(handle)CloseHandle(handle);}} mutex{testing?nullptr:CreateMutexW(nullptr,FALSE,L"Local\\HavenDesktopApp")};

    // Read before creating the multimedia backend. These Qt 6.12 switches are private APIs.
    if(QSettings().value("compatibilityPlayback",false).toBool() || app.arguments().contains("--safe-playback")) {
        qputenv("HAVEN_SAFE_PLAYBACK","1");
        qputenv("QT_FFMPEG_DECODING_HW_DEVICE_TYPES",",");
        qputenv("QT_DISABLE_HW_TEXTURES_CONVERSION","1");
    }
    PlayerWindow window;
    window.show();
    // The event loop dispatches clicks, paint requests, and media notifications.
    return app.exec();
}
