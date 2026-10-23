#ifndef TST_EXTRACOLUMNSPROXYMODEL_H
#define TST_EXTRACOLUMNSPROXYMODEL_H

#include <QObject>

class QStandardItemModel;
class ExtraColumnsProxyModel;

class TestExtraColumnsProxyModel : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void modelTester();
    void extraColumnsAfterSourceColumns();
    void extraColumnsReadOnlyWithoutSource();
    void extraColumnsHaveNoChildren();
    void rowSelectionMapsToSourceColumnsOnly();
    void extraColumnsFollowSourceColumnInsert();
    void layoutChangeKeepsExtraColumnIndexOnItsRow();
    void extraColumnsChangedEmitsForExtraColumnsOnly();

private:
    QStandardItemModel* _pSource{ nullptr };
    ExtraColumnsProxyModel* _pProxy{ nullptr };
};

#endif // TST_EXTRACOLUMNSPROXYMODEL_H
