#pragma once
#include <QWidget>
#include <QDialog>
class ProductivityModel;
class QListWidget;
class QLabel;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QMouseEvent;
class QCloseEvent;
class QCheckBox;
class QComboBox;
class QTabWidget;

class ProductivityPanel final : public QWidget {
    Q_OBJECT
public:
    explicit ProductivityPanel(ProductivityModel* model,QWidget* parent=nullptr);
signals:
    void toolRequested(int tool);
private:
    void refreshIntentions();void refreshClocks();
    ProductivityModel* model_;
    QListWidget* tasks_;QListWidget* goals_;
    QLabel* progressText_;QProgressBar* progress_;
    QLabel* timerText_;QLabel* stopwatchText_;QLabel* pomodoroText_;QLabel* phaseText_;QLabel* roundsText_;
    QPushButton* timerStart_;QPushButton* stopwatchStart_;QPushButton* pomodoroStart_;
    QSpinBox* timerMinutes_;QSpinBox* focus_;QSpinBox* short_;QSpinBox* long_;
};


class ProductivityDock final : public QDialog {
    Q_OBJECT
public:
    explicit ProductivityDock(ProductivityModel* model,QWidget* parent=nullptr);
    ~ProductivityDock() override;
    void reveal();
    void revealIfActive();
    bool desktopVisible() const;
    void setOverlayEnabled(bool enabled);
    void setTasksVisible(bool visible);
    void setGoalsVisible(bool visible);
    void setCorner(const QString& corner);
    bool overlayEnabled() const {return enabled_;}
    bool tasksVisible() const {return showTasks_;}
    bool goalsVisible() const {return showGoals_;}
    QString corner() const {return corner_;}
    int activeClockCount() const;
protected:
    void closeEvent(QCloseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private:
    void refresh();void refreshLists();void anchor();void savePreferences();
    ProductivityModel* model_;
    QWidget* clockTiles_[3]{};
    QLabel* clockTimes_[3]{};
    QLabel* clockNames_[3]{};
    QWidget* taskTile_;
    QWidget* goalTile_;
    class QVBoxLayout* taskRows_;
    class QVBoxLayout* goalRows_;
    QString corner_="Top right";
    bool enabled_=true,showTasks_=false,showGoals_=false;
    QPoint dragOffset_;
    bool dragging_=false,customPosition_=false;
};
