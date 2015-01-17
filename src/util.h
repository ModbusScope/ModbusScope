#ifndef UTIL_H
#define UTIL_H

#include "QObject"
#include "QLocale"

class Util : public QObject
{
    Q_OBJECT

public:

    static QChar getSeparatorCharacter()
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

    static QString formatDoubleForExport(double number)
    {
        QString tmp;

        // Round number to 3 decimals and remove group separator
        tmp = QLocale::system().toString(number, 'f', 3).replace(QLocale::system().groupSeparator(), "");

        // convert back to double
        double doubleTmp = QLocale::system().toDouble(tmp);

        // Make sure trailing zeros are removed and remove group separator
        tmp = QLocale::system().toString(doubleTmp, 'g', 9).replace(QLocale::system().groupSeparator(), "");

        return tmp;
    }

private:

};


#endif // UTIL_H
