#pragma once

#include <QMainWindow>
#include <QByteArray>
#include <memory>

class QMediaPlayer;
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
    void importYoutube();
    void saveLibrary();
    void refreshMonitors();
    void quitApplication();

    QMediaPlayer* player_;
    QPushButton* playButton_;
    QAudioOutput* audio_;
    QVideoWidget* preview_;
    QVideoWidget* wallpaper_;
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
    ProductivityModel* productivity_;
    ProductivityDock* dayBar_;
    YouTubeSetup* youtubeSetup_;
};
