#pragma once
#include <QString>
#include <QUrl>
namespace ImportSupport {
QUrl normalizeYoutube(const QString& input);
QString formatSelector();
QString completedPath(const QString& output,const QString& folder);
QString failureMessage(const QString& output,int exitCode,bool crashed,bool cancelled);
}
