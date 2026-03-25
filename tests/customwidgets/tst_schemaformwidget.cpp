#include "tst_schemaformwidget.h"

#include "customwidgets/schemaformwidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineEdit>
#include <QSpinBox>
#include <QTest>

namespace {

//! Build a \c "type":"object" schema with a single property.
QJsonObject makeObjectSchema(const QString& key, const QJsonObject& propSchema)
{
    QJsonObject props;
    props[key] = propSchema;

    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = props;
    return schema;
}

} // namespace

void TestSchemaFormWidget::emptySchemaCreatesEmptyForm()
{
    SchemaFormWidget w;
    w.setSchema(QJsonObject(), QJsonObject());

    QVERIFY(w.values().isEmpty());
    QVERIFY(w.findChild<QLineEdit*>() == nullptr);
    QVERIFY(w.findChild<QSpinBox*>() == nullptr);
    QVERIFY(w.findChild<QCheckBox*>() == nullptr);
    QVERIFY(w.findChild<QComboBox*>() == nullptr);
}

void TestSchemaFormWidget::stringPropertyCreatesLineEdit()
{
    QJsonObject propSchema;
    propSchema["type"] = "string";

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("host", propSchema), QJsonObject{ { "host", "localhost" } });

    auto* edit = w.findChild<QLineEdit*>();
    QVERIFY(edit != nullptr);
    QCOMPARE(edit->text(), QStringLiteral("localhost"));
    QCOMPARE(w.values()["host"].toString(), QStringLiteral("localhost"));
}

void TestSchemaFormWidget::integerPropertyCreatesSpinBox()
{
    QJsonObject propSchema;
    propSchema["type"] = "integer";

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("port", propSchema), QJsonObject{ { "port", 502 } });

    auto* spin = w.findChild<QSpinBox*>();
    QVERIFY(spin != nullptr);
    QCOMPARE(spin->value(), 502);
    QCOMPARE(w.values()["port"].toInt(), 502);
}

void TestSchemaFormWidget::numberPropertyCreatesDoubleSpinBox()
{
    QJsonObject propSchema;
    propSchema["type"] = "number";

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("scale", propSchema), QJsonObject{ { "scale", 1.5 } });

    auto* dspin = w.findChild<QDoubleSpinBox*>();
    QVERIFY(dspin != nullptr);
    QCOMPARE(dspin->value(), 1.5);
    QCOMPARE(w.values()["scale"].toDouble(), 1.5);
}

void TestSchemaFormWidget::booleanPropertyCreatesCheckBox()
{
    QJsonObject propSchema;
    propSchema["type"] = "boolean";

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("enabled", propSchema), QJsonObject{ { "enabled", true } });

    auto* check = w.findChild<QCheckBox*>();
    QVERIFY(check != nullptr);
    QCOMPARE(check->isChecked(), true);
    QCOMPARE(w.values()["enabled"].toBool(), true);
}

void TestSchemaFormWidget::stringEnumCreatesComboBox()
{
    QJsonObject propSchema;
    propSchema["type"] = "string";
    propSchema["enum"] = QJsonArray{ "N", "E", "O" };

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("parity", propSchema), QJsonObject{ { "parity", "E" } });

    auto* combo = w.findChild<QComboBox*>();
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->currentText(), QStringLiteral("E"));
    QCOMPARE(w.values()["parity"].toString(), QStringLiteral("E"));
}

void TestSchemaFormWidget::integerEnumCreatesComboBox()
{
    QJsonObject propSchema;
    propSchema["type"] = "integer";
    propSchema["enum"] = QJsonArray{ 9600, 19200, 115200 };

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("baudrate", propSchema), QJsonObject{ { "baudrate", 19200 } });

    auto* combo = w.findChild<QComboBox*>();
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->currentText(), QStringLiteral("19200"));

    QJsonValue result = w.values()["baudrate"];
    QCOMPARE(result.toInt(), 19200);
    QVERIFY(result.type() == QJsonValue::Double); // JSON has no int/double distinction
}

void TestSchemaFormWidget::integerSchemaMinMaxApplied()
{
    QJsonObject propSchema;
    propSchema["type"] = "integer";
    propSchema["minimum"] = 1;
    propSchema["maximum"] = 247;

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("slaveId", propSchema), QJsonObject{ { "slaveId", 5 } });

    auto* spin = w.findChild<QSpinBox*>();
    QVERIFY(spin != nullptr);
    QCOMPARE(spin->minimum(), 1);
    QCOMPARE(spin->maximum(), 247);
    QCOMPARE(spin->value(), 5);
}

void TestSchemaFormWidget::valuesReflectWidgetChanges()
{
    QJsonObject propSchema;
    propSchema["type"] = "string";

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("name", propSchema), QJsonObject{ { "name", "original" } });

    auto* edit = w.findChild<QLineEdit*>();
    QVERIFY(edit != nullptr);
    edit->setText("changed");

    QCOMPARE(w.values()["name"].toString(), QStringLiteral("changed"));
}

void TestSchemaFormWidget::setSchemaResetsOldWidgets()
{
    SchemaFormWidget w;

    QJsonObject stringProp;
    stringProp["type"] = "string";
    w.setSchema(makeObjectSchema("host", stringProp), QJsonObject());

    QVERIFY(w.findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly) != nullptr);
    QVERIFY(w.findChild<QSpinBox*>(QString(), Qt::FindDirectChildrenOnly) == nullptr);

    QJsonObject intProp;
    intProp["type"] = "integer";
    w.setSchema(makeObjectSchema("port", intProp), QJsonObject());

    QVERIFY(w.findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly) == nullptr);
    QVERIFY(w.findChild<QSpinBox*>(QString(), Qt::FindDirectChildrenOnly) != nullptr);
}

QTEST_MAIN(TestSchemaFormWidget)
