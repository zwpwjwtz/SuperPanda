#ifndef GSETTINGSEDITOR_H
#define GSETTINGSEDITOR_H

#include <QVariant>


class GSettingsEditor
{
public:
    GSettingsEditor();

    static bool existKey(QString schema, QString key);
    static QVariant getValue(QString schema, QString key);
    static bool setValue(QString schema, QString key, const QVariant& value);
    static bool resetValue(QString schema, QString key);
};

#endif // GSETTINGSEDITOR_H
