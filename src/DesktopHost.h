#pragma once
#include <QString>
#include <QtGui/qwindowdefs.h>
class DesktopHost {
public:
    static bool attach(WId window, const QString& monitor, QString& error);
    static void detach(WId window);
    static bool attached(WId window);
};
