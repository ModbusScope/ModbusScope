
#include <QObject>


class TestProjectFileParser: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void legacyRegExpressions();
    void newRegExpressions();
    void bothLegacyNewRegExpressions();

private:

};
