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
        QString tmp;
        
        /* TODO: Can't this be done without converting to string and back and back again? */

        // Round number to 3 decimals
        tmp = QLocale().toString(number, 'f', 3);

        // Remove group separator
        if (QLocale().groupSeparator() != QLocale().decimalPoint())
        {
            tmp = tmp.replace(QLocale().groupSeparator(), "");
        }

        // convert back to double
        double doubleTmp = QLocale().toDouble(tmp);

        // Make sure trailing zeros are removed
        tmp = QLocale().toString(doubleTmp, 'g', 9);

        // Remove group separator
        if (QLocale().groupSeparator() != QLocale().decimalPoint())
        {
            tmp = tmp.replace(QLocale().groupSeparator(), "");
        }

        return tmp;
    }
    
    static const QList<QColor> cColorlist;

private:

};


#endif // UTIL_H
