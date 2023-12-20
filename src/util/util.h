#ifndef UTIL_H
#define UTIL_H

#include <QMessageBox>
#include <QLocale>
#include <QColor>
#include "version.h"

class Util : public QObject
{
    Q_OBJECT

public:

    static QChar separatorCharacter()
    {
        if (QLocale().decimalPoint() == ',')
        {
            return ';';
        }
        else
        {
            return ',';
        }
    }

    static void showError(QString text)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("ModbusScope"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(text);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    static const QString & currentVersion()
    {
        static const QString version(APP_VERSION);
        return version;
    }

    static QString formatDoubleForExport(double number)
    {
        QLocale locale = QLocale();
        locale.setNumberOptions(QLocale::OmitGroupSeparator);

        QString str;
        if (number < 0.0001)
        {
            str = locale.toString(number, 'g', QLocale::FloatingPointShortest);
        }
        else
        {
            str = locale.toString(number, 'F', QLocale::FloatingPointShortest);
        }
        return str;
    }
    
    static const QList<QColor> cColorlist;

private:

};


#endif // UTIL_H
