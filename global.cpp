#include "global.h"
#include <QTranslator>


// Global object definitions
MagicKonfug* windowMgckf = nullptr;
TedyGuard* windowTdgrd = nullptr;
AboutWindow* windowAbout = nullptr;
QTranslator appTranslator;

const char* spanda_language_string[4] =
{
    "en_US",
    "fr_FR",
    "zh_CN",
    "zh_TW"
};
