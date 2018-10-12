#ifndef COMMON_H
#define COMMON_H

#include <QCoreApplication>


// Global object declarations
class MagicKonfug;
class TedyGuard;
class AboutWindow;

extern MagicKonfug* windowMgckf;
extern TedyGuard* windowTdgrd;
extern AboutWindow* windowAbout;
extern QTranslator appTranslator;

extern const char* spanda_language_string[4];

#endif // COMMON_H
