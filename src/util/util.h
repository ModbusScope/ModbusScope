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
        if (QLocale::system().decimalPoint() == ',')
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
        tmp = QLocale::system().toString(number, 'f', 3);

        // Remove group separator
        if (QLocale::system().groupSeparator() != QLocale::system().decimalPoint())
        {
            tmp = tmp.replace(QLocale::system().groupSeparator(), "");
        }

        // convert back to double
        double doubleTmp = QLocale::system().toDouble(tmp);

        // Make sure trailing zeros are removed
        tmp = QLocale::system().toString(doubleTmp, 'g', 9);

        // Remove group separator
        if (QLocale::system().groupSeparator() != QLocale::system().decimalPoint())
        {
            tmp = tmp.replace(QLocale::system().groupSeparator(), "");
        }

        return tmp;
    }
    
    static const QList<QColor> cColorlist;

private:

};


#endif // UTIL_H
