#pragma once

#include <QMainWindow>
#include <QByteArray>
#include <memory>

class QTimer;
class MediaController;
class QPushButton;
class QAudioOutput;
class QVideoWidget;
class QListWidget;
class QLineEdit;
class QComboBox;
class QLabel;
class QProcess;
class QSystemTrayIcon;
class QCloseEvent;
class ProductivityModel;
class ProductivityDock;
class ImportJob;
class YouTubeSetup;

class PlayerWindow final : public QMainWindow
{
public:
    explicit PlayerWindow(QWidget* parent = nullptr);
    ~PlayerWindow() override;
protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void openVideo();
    void togglePlayback();
    void addVideo(const QString& path);
    void playSelected(bool wallpaper);
    void removeWallpaper();
    void failPlayback(const QString& message);
    void acceptVideoFrame();
    void importYoutube();
    void saveLibrary();
    void refreshMonitors();
    void quitApplication();

    MediaController* player_;
    QPushButton* playButton_;
    QAudioOutput* audio_;
    QWidget* preview_;
    QWidget* wallpaper_;
    QPushButton* importButton_;
    QListWidget* library_;
    QLineEdit* url_;
    QComboBox* monitors_;
    QLabel* state_;
    QProcess* importer_;
    QSystemTrayIcon* tray_;
    QByteArray importOutput_;
    QString importPath_;
    QString importFolder_;
    bool importCancelled_=false;
    std::unique_ptr<ImportJob> importJob_;
    bool desktopActive_=false;
    bool pendingWallpaper_=false;
    bool waitingForFrame_=false;
    bool loadingLibrary_=false;
    bool testing_=false;
    QString requestedMonitor_;
    QTimer* playbackDeadline_;
    qint64 lastFrameAt_=0;
    quint64 playbackRequest_=0;
    ProductivityModel* productivity_;
    ProductivityDock* dayBar_;
    YouTubeSetup* youtubeSetup_;
};
