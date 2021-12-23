
#include <QObject>

#include "projectfiledata.h"

class TestUpdateRegisterNewExpression: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void tags();
    void tags_signed32();
    void tags_signed16();
    void tags_connection();
    void tags_signed16_connection();

private:
    void checkOperation(ProjectFileData::RegisterSettings& regSettings, QString regResult);

};
