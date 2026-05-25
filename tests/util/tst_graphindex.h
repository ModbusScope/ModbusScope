
#ifndef TST_GRAPHINDEX_H
#define TST_GRAPHINDEX_H

#include <QObject>

class TestGraphIndex : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void graphIdxDefaultConstructedIsInvalid();
    void graphIdxExplicitValueIsValid();
    void graphIdxExplicitValueStoresValue();
    void graphIdxNoneConstantMatchesDefault();

    void graphIdxEqualitySameValue();
    void graphIdxEqualityDifferentValues();
    void graphIdxInequality();
    void graphIdxLessThan();

    void activeIdxDefaultConstructedIsInvalid();
    void activeIdxExplicitValueIsValid();
    void activeIdxExplicitValueStoresValue();
    void activeIdxNoneConstantMatchesDefault();

    void activeIdxEqualitySameValue();
    void activeIdxEqualityDifferentValues();
    void activeIdxInequality();
    void activeIdxLessThan();

    void metatype_graphIdx_isRegistered();
    void metatype_activeIdx_isRegistered();
};

#endif /* TST_GRAPHINDEX_H */
