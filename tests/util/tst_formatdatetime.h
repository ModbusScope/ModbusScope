
#ifndef TST_FORMATDATETIME_H
#define TST_FORMATDATETIME_H

#include <QObject>

class TestFormatDateTime : public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();

    void dateStringFormat();
    void timeStringFormat();
    void dateTimeStringFormat();
    void formatDateTime();
    void currentDateTime();

private:
};

#endif /* TST_FORMATDATETIME_H */
