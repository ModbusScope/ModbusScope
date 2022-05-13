
#include <QObject>

class TestFormatRelativeTime: public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();
    void init();
    void cleanup();

    void formatTimeSmallScale_data();
    void formatTimeSmallScale();

    void formatTime_data();
    void formatTime();

    void formatTime_Absolute_data();
    void formatTime_Absolute();

private:

};
