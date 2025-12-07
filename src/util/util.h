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
        auto locale = QLocale();
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

    /*!
     * Calculate the least number of decimals to represent a number between minVal and maxVal
     */
    static int decimalsFromRange(double minVal, double maxVal)
    {
        double range = std::fabs(maxVal - minVal);
        if (range == 0)
        {
            return 0;
        }

        int decimals = static_cast<int>(std::floor(std::log10(range)));
        if (decimals >= 0)
        {
            return 0;
        }
        else
        {
            decimals = std::abs(decimals);
        }

        return decimals;
    }

    /*!
     * Round a double to a given number of decimals
     */
    static double roundToDecimals(double value, int decimals)
    {
        double factor = std::pow(10.0, decimals);
        return std::round(value * factor) / factor;
    }

    static const QList<QColor> cColorlist;

private:

};


#endif // UTIL_H
