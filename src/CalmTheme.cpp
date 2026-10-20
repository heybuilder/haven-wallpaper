#include "CalmTheme.h"
QString calmStyle(const QString& theme){
    QString background="#f2f3ed",card="#fffdf8",text="#35433a",muted="#606e62",soft="#dde5d8",accent="#526c58",line="#d5dbcf";
    if(theme=="Rose linen"){background="#f6efec";card="#fffbf8";text="#523e43";muted="#7a6267";soft="#efddda";accent="#80575f";line="#e6d6d3";}
    if(theme=="Lavender dusk"){background="#f0edf5";card="#fcfaff";text="#464056";muted="#706780";soft="#e5deef";accent="#675b80";line="#dcd4e6";}
    return QString(
        "QWidget{font-family:'Segoe UI';font-size:13px;color:%3;background:transparent;}"
        "QMainWindow,QDialog{background:%1;}"
        "QFrame#card{background:%2;border:1px solid %7;border-radius:14px;}"
        "QLabel#heading{font-family:Georgia;font-size:30px;font-weight:normal;}"
        "QLabel#eyebrow{font-size:11px;font-weight:600;letter-spacing:2px;color:%4;}"
        "QLabel#muted{color:%4;}QLabel#clock{font-size:42px;font-weight:300;}"
        "QPushButton{background:%5;border:1px solid %7;border-radius:8px;padding:9px 13px;}"
        "QPushButton:hover{background:%7;}QPushButton:pressed{background:%5;}"
        "QPushButton[primary=\"true\"]{background:%6;color:white;border:0;}"
        "QPushButton:disabled{color:%4;background:%1;}"
        "QLineEdit,QListWidget,QComboBox,QSpinBox{background:%2;border:1px solid %7;border-radius:8px;padding:8px;}"
        "QLineEdit:focus,QListWidget:focus,QSpinBox:focus{border:1px solid %6;}"
        "QListWidget::item{padding:9px;border-radius:5px;}"
        "QListWidget::item:selected{background:%5;color:%3;}"
        "QTabWidget::pane{border:0;background:%1;}QTabBar::tab{background:transparent;color:%4;padding:10px 18px;margin:2px;}"
        "QTabBar::tab:selected{background:%5;color:%3;border-radius:8px;}"
        "QProgressBar{border:0;border-radius:4px;background:%5;height:6px;text-align:center;}"
        "QProgressBar::chunk{background:%6;border-radius:4px;}"
        "QSlider::groove:horizontal{height:5px;background:%7;border-radius:2px;}"
        "QSlider::handle:horizontal{background:%6;width:14px;margin:-5px 0;border-radius:7px;}"
        "QCheckBox{spacing:7px;}QMenu{background:%2;border:1px solid %7;padding:6px;}"
        "QMenu::item{padding:8px 18px;}QMenu::item:selected{background:%5;}"
        "QToolTip{background:%2;color:%3;border:1px solid %7;}"
        "QScrollArea{border:0;}"
    ).arg(background,card,text,muted,soft,accent,line);
}
