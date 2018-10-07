#include "gsettingseditor.h"
#include "../Interfaces/exelauncher.h"

#define SPANDA_GSETTINGS_EXEC_DEFAULT "gsettings"
#define SPANDA_GSETTINGS_CMD_KEY_LIST "list-keys"
#define SPANDA_GSETTINGS_CMD_VAR_GET "get"
#define SPANDA_GSETTINGS_CMD_VAR_SET "set"
#define SPANDA_GSETTINGS_CMD_VAR_RESET "reset"


GSettingsEditor::GSettingsEditor()
{
}

bool GSettingsEditor::existKey(QString schema, QString key)
{
    ExeLauncher exe;
    exe.runCommand(QString("%1 %2 %3")
                          .arg(SPANDA_GSETTINGS_EXEC_DEFAULT)
                          .arg(SPANDA_GSETTINGS_CMD_KEY_LIST)
                          .arg(schema),
                   true);

    QList<QByteArray> output(exe.getOutput().split('\n'));
    return output.contains(key.toUtf8());
}

QVariant GSettingsEditor::getValue(QString schema, QString key)
{
    ExeLauncher exe;
    exe.runCommand(QString("%1 %2 %3 %4")
                          .arg(SPANDA_GSETTINGS_EXEC_DEFAULT)
                          .arg(SPANDA_GSETTINGS_CMD_VAR_GET)
                          .arg(schema).arg(key),
                   true);
    if (!(exe.getErrCode() == ExeLauncher::ExecOk && exe.getExitCode() == 0))
        return QVariant();

    QByteArray output(exe.getOutput());
    QString temp;
    QVariant result;
    bool conversionOK = true;

    // Try to parse variable type from the output
    switch (output.at(0))
    {
        case '[': // Array:
            // Currently, only string arrays are supported
            temp = QString::fromUtf8(output.mid(1, output.indexOf(']' - 1)));
            if (output.at(1) == '\'')
            {
                // Array of strings
                result.setValue(temp.split("', '"));
            }
            else
                conversionOK = false;
            break;
        case '\'': // String
            temp = QString::fromUtf8(
                                output.mid(1, output.lastIndexOf('\'' - 1)));
            result.setValue(temp);
            break;
        case 't':
        case 'f': // Boolean
            if (output.indexOf("true") == 0)
                result.setValue(bool(true));
            else if (output.indexOf("false") == 0)
                result.setValue(bool(false));
            else
                conversionOK = false;
            break;
        default: // Numeric
            temp = output.left(output.indexOf(' ')); // Sub-type string
            output = output.remove(0, output.indexOf(' ') + 1).trimmed();
            if (temp.contains("uint")) // Unsigned integer
                result.setValue(output.toUInt(&conversionOK));
            else // Other types of value: try to convert it
            {
                output.toInt(&conversionOK);
                if (conversionOK)
                    result.setValue(output.toInt(&conversionOK));
                else
                    result.setValue(output.toDouble(&conversionOK));
            }
            break;
    }

    if (!conversionOK)
        result.clear();
    return result;
}

bool GSettingsEditor::setValue(QString schema, QString key, const QVariant &value)
{
    QString valueString = value.toString();
    ExeLauncher exe;
    exe.runCommand(QString("%1 %2 %3 %4 %5")
                          .arg(SPANDA_GSETTINGS_EXEC_DEFAULT)
                          .arg(SPANDA_GSETTINGS_CMD_VAR_SET)
                          .arg(schema).arg(key).arg(valueString),
                   true);

    return (exe.getErrCode() == ExeLauncher::ExecOk && exe.getExitCode() == 0);
}

bool GSettingsEditor::resetValue(QString schema, QString key)
{
    ExeLauncher exe;
    exe.runCommand(QString("%1 %2 %3 %4")
                          .arg(SPANDA_GSETTINGS_EXEC_DEFAULT)
                          .arg(SPANDA_GSETTINGS_CMD_VAR_RESET)
                          .arg(schema).arg(key),
                   true);

    return (exe.getErrCode() == ExeLauncher::ExecOk && exe.getExitCode() == 0);
}
