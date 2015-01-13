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

private:

};


#endif // UTIL_H
