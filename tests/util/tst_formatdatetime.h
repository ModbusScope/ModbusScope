
#ifndef TEST_FORMATDATETIME_H__
#define TEST_FORMATDATETIME_H__

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

#endif /* TEST_FORMATDATETIME_H__ */
