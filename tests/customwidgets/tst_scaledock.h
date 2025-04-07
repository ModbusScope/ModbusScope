
#include <QObject>

class ScaleDock;

class TestScaleDock: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void yAxis1SelectedOnBoot();

private:

    ScaleDock* _pScaleDock;

};
