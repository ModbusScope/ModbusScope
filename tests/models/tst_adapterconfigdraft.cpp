#include "tst_adapterconfigdraft.h"

#include "models/adapterconfigdraft.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QSignalSpy>
#include <QTest>

namespace {

QJsonArray namedItems(const QStringList& names)
{
    QJsonArray items;
    for (const QString& name : names)
    {
        items.append(QJsonObject{ { "name", name } });
    }
    return items;
}

} // namespace

void TestAdapterConfigDraft::configFallsBackToSavedConfig()
{
    SettingsModel model;
    model.setAdapterCurrentConfig("a", QJsonObject{ { "connections", namedItems({ "Saved" }) } });
    AdapterConfigDraft draft(&model);

    QCOMPARE(draft.config("a").value("connections").toArray(), namedItems({ "Saved" }));
}

void TestAdapterConfigDraft::sectionOverridesSavedKey()
{
    SettingsModel model;
    model.setAdapterCurrentConfig("a", QJsonObject{ { "connections", namedItems({ "Saved" }) }, { "other", 1 } });
    AdapterConfigDraft draft(&model);

    draft.setSection("a", "connections", namedItems({ "Edited" }));

    QCOMPARE(draft.config("a").value("connections").toArray(), namedItems({ "Edited" }));
    QCOMPARE(draft.config("a").value("other").toInt(), 1);
}

void TestAdapterConfigDraft::sectionsOfDifferentKeysAreKept()
{
    SettingsModel model;
    model.setAdapterCurrentConfig("a", QJsonObject{ { "connections", namedItems({ "Saved" }) } });
    AdapterConfigDraft draft(&model);

    draft.setSection("a", "connections", namedItems({ "Edited" }));
    draft.setSection("a", "general", QJsonObject{ { "poll", 500 } });

    QCOMPARE(draft.config("a").value("connections").toArray(), namedItems({ "Edited" }));
    QCOMPARE(draft.config("a").value("general").toObject().value("poll").toInt(), 500);
}

void TestAdapterConfigDraft::sectionsOfDifferentAdaptersAreIndependent()
{
    SettingsModel model;
    model.setAdapterCurrentConfig("a", QJsonObject{ { "connections", namedItems({ "A" }) } });
    model.setAdapterCurrentConfig("b", QJsonObject{ { "connections", namedItems({ "B" }) } });
    AdapterConfigDraft draft(&model);

    draft.setSection("a", "connections", namedItems({ "A edited" }));

    QCOMPARE(draft.config("b").value("connections").toArray(), namedItems({ "B" }));
}

void TestAdapterConfigDraft::setSectionEmitsConfigChanged()
{
    SettingsModel model;
    AdapterConfigDraft draft(&model);
    QSignalSpy spy(&draft, &AdapterConfigDraft::configChanged);

    draft.setSection("a", "connections", namedItems({ "X" }));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("a"));
}

void TestAdapterConfigDraft::unchangedSectionDoesNotEmit()
{
    SettingsModel model;
    AdapterConfigDraft draft(&model);
    draft.setSection("a", "connections", namedItems({ "X" }));
    QSignalSpy spy(&draft, &AdapterConfigDraft::configChanged);

    draft.setSection("a", "connections", namedItems({ "X" }));

    QCOMPARE(spy.count(), 0);
}

void TestAdapterConfigDraft::modelUpdateEmitsConfigChangedAndKeepsSections()
{
    SettingsModel model;
    AdapterConfigDraft draft(&model);
    draft.setSection("a", "connections", namedItems({ "Edited" }));
    QSignalSpy spy(&draft, &AdapterConfigDraft::configChanged);

    model.setAdapterCurrentConfig("a", QJsonObject{ { "connections", namedItems({ "Saved" }) } });

    QCOMPARE(spy.count(), 1);
    QCOMPARE(draft.config("a").value("connections").toArray(), namedItems({ "Edited" }));
}

void TestAdapterConfigDraft::draftDoesNotWriteModel()
{
    SettingsModel model;
    model.setAdapterCurrentConfig("a", QJsonObject{ { "connections", namedItems({ "Saved" }) } });
    AdapterConfigDraft draft(&model);

    draft.setSection("a", "connections", namedItems({ "Edited" }));

    QCOMPARE(model.adapterData("a")->effectiveConfig().value("connections").toArray(), namedItems({ "Saved" }));
}

QTEST_MAIN(TestAdapterConfigDraft)
