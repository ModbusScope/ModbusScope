
#include <QObject>

class TestUtil: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void formatDouble_data();
    void formatDouble();

private:


};
