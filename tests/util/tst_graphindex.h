
#ifndef TST_GRAPHINDEX_H
#define TST_GRAPHINDEX_H

#include <QObject>

class TestGraphIndex : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void graphIdx_defaultConstructed_isInvalid();
    void graphIdx_explicitValue_isValid();
    void graphIdx_explicitValue_storesValue();
    void graphIdx_noneConstant_matchesDefault();

    void graphIdx_equality_sameValue();
    void graphIdx_equality_differentValues();
    void graphIdx_inequality();
    void graphIdx_lessThan();

    void activeIdx_defaultConstructed_isInvalid();
    void activeIdx_explicitValue_isValid();
    void activeIdx_explicitValue_storesValue();
    void activeIdx_noneConstant_matchesDefault();

    void activeIdx_equality_sameValue();
    void activeIdx_equality_differentValues();
    void activeIdx_inequality();
    void activeIdx_lessThan();

    void metatype_graphIdx_isRegistered();
    void metatype_activeIdx_isRegistered();
};

#endif /* TST_GRAPHINDEX_H */
