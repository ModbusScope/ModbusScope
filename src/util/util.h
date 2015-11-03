#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <QLocale>
#include <QColor>

class Util : public QObject
{
    Q_OBJECT

public:

    static const quint32 cCurrentDataLevel = 2;

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

    static quint32 currentDataLevel()
    {
        return cCurrentDataLevel;
    }

    static const QString & currentVersion()
    {
        static const QString version(APP_VERSION);
        return version;
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

    static const QList<QColor> cColorlist;

private:

};


#endif // UTIL_H
