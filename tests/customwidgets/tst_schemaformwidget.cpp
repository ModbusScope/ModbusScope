#include "tst_schemaformwidget.h"

#include "customwidgets/schemaformwidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
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

/*!
 * \brief Build a schema with base properties (\c id, \c type) and an if/then/else block.
 *
 * if type == "tcp": then shows \c ip (string) and \c port (integer).
 * else:             shows \c portName (string) and \c baudrate (integer enum).
 */
QJsonObject makeConditionalSchema()
{
    QJsonObject idProp;
    idProp["type"] = "integer";
    idProp["title"] = "ID";

    QJsonObject typeProp;
    typeProp["type"] = "string";
    typeProp["title"] = "Type";
    typeProp["enum"] = QJsonArray{ "tcp", "serial" };

    QJsonObject baseProps;
    baseProps["id"] = idProp;
    baseProps["type"] = typeProp;

    QJsonObject ifConstObj;
    ifConstObj["const"] = "tcp";

    QJsonObject ifPropsInner;
    ifPropsInner["type"] = ifConstObj;

    QJsonObject ifObj;
    ifObj["properties"] = ifPropsInner;
    ifObj["required"] = QJsonArray{ "type" };

    QJsonObject ipProp;
    ipProp["type"] = "string";
    ipProp["title"] = "IP Address";

    QJsonObject portProp;
    portProp["type"] = "integer";
    portProp["title"] = "Port";

    QJsonObject thenProps;
    thenProps["ip"] = ipProp;
    thenProps["port"] = portProp;

    QJsonObject thenObj;
    thenObj["properties"] = thenProps;

    QJsonObject portNameProp;
    portNameProp["type"] = "string";
    portNameProp["title"] = "Port Name";

    QJsonObject baudrateProp;
    baudrateProp["type"] = "integer";
    baudrateProp["enum"] = QJsonArray{ 9600, 115200 };

    QJsonObject elseProps;
    elseProps["portName"] = portNameProp;
    elseProps["baudrate"] = baudrateProp;

    QJsonObject elseObj;
    elseObj["properties"] = elseProps;

    QJsonObject schema;
    schema["type"] = "object";
    schema["properties"] = baseProps;
    schema["if"] = ifObj;
    schema["then"] = thenObj;
    schema["else"] = elseObj;
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
    const QJsonObject hostResult = w.values();
    QCOMPARE(hostResult["host"].toString(), QStringLiteral("localhost"));
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
    const QJsonObject portResult = w.values();
    QCOMPARE(portResult["port"].toInt(), 502);
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
    const QJsonObject scaleResult = w.values();
    QCOMPARE(scaleResult["scale"].toDouble(), 1.5);
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
    const QJsonObject enabledResult = w.values();
    QCOMPARE(enabledResult["enabled"].toBool(), true);
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
    const QJsonObject parityResult = w.values();
    QCOMPARE(parityResult["parity"].toString(), QStringLiteral("E"));
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

    const QJsonObject baudrateResult = w.values();
    const QJsonValue result = baudrateResult["baudrate"];
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

    const QJsonObject nameResult = w.values();
    QCOMPARE(nameResult["name"].toString(), QStringLiteral("changed"));
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

void TestSchemaFormWidget::stringEnumWithLabelsShowsLabel()
{
    QJsonObject propSchema;
    propSchema["type"] = "string";
    propSchema["enum"] = QJsonArray{ "N", "E", "O" };
    propSchema["x-enumLabels"] = QJsonArray{ "None", "Even", "Odd" };

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("parity", propSchema), QJsonObject{ { "parity", "E" } });

    auto* combo = w.findChild<QComboBox*>();
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->currentText(), QStringLiteral("Even"));
    const QJsonObject parityLabelsResult = w.values();
    QCOMPARE(parityLabelsResult["parity"].toString(), QStringLiteral("E"));
}

void TestSchemaFormWidget::integerEnumWithLabelsShowsLabel()
{
    QJsonObject propSchema;
    propSchema["type"] = "integer";
    propSchema["enum"] = QJsonArray{ 1, 2, 3 };
    propSchema["x-enumLabels"] = QJsonArray{ "1", "2", "1.5" };

    SchemaFormWidget w;
    w.setSchema(makeObjectSchema("stopbits", propSchema), QJsonObject{ { "stopbits", 3 } });

    auto* combo = w.findChild<QComboBox*>();
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->currentText(), QStringLiteral("1.5"));
    const QJsonObject stopbitsResult = w.values();
    QCOMPARE(stopbitsResult["stopbits"].toInt(), 3);
}

void TestSchemaFormWidget::conditionalTcpFieldsVisibleOnLoad()
{
    SchemaFormWidget w;
    QJsonObject values;
    values["type"] = "tcp";
    values["ip"] = "127.0.0.1";
    values["port"] = 502;
    w.setSchema(makeConditionalSchema(), values);

    const QJsonObject result = w.values();
    QVERIFY(result.contains("ip"));
    QVERIFY(result.contains("port"));
    QVERIFY(!result.contains("portName"));
    QVERIFY(!result.contains("baudrate"));
}

void TestSchemaFormWidget::conditionalSerialFieldsVisibleOnLoad()
{
    SchemaFormWidget w;
    QJsonObject values;
    values["type"] = "serial";
    values["portName"] = "/dev/ttyS0";
    values["baudrate"] = 9600;
    w.setSchema(makeConditionalSchema(), values);

    const QJsonObject result = w.values();
    QVERIFY(result.contains("portName"));
    QVERIFY(result.contains("baudrate"));
    QVERIFY(!result.contains("ip"));
    QVERIFY(!result.contains("port"));
}

void TestSchemaFormWidget::conditionalSwitchShowsSerialHidesTcp()
{
    SchemaFormWidget w;
    QJsonObject values;
    values["type"] = "tcp";
    values["ip"] = "127.0.0.1";
    values["port"] = 502;
    w.setSchema(makeConditionalSchema(), values);

    // Find the type combo — the one that has "tcp" as a data item
    QComboBox* typeCombo = nullptr;
    const auto combos = w.findChildren<QComboBox*>();
    for (QComboBox* combo : combos)
    {
        if (combo->findData(QStringLiteral("tcp")) >= 0)
        {
            typeCombo = combo;
            break;
        }
    }
    QVERIFY(typeCombo != nullptr);
    if (typeCombo == nullptr)
    {
        return;
    }
    typeCombo->setCurrentIndex(typeCombo->findData(QStringLiteral("serial")));

    const QJsonObject result = w.values();
    QVERIFY(result.contains("portName"));
    QVERIFY(!result.contains("ip"));
    QVERIFY(!result.contains("port"));
}

void TestSchemaFormWidget::conditionalSwitchShowsTcpHidesSerial()
{
    SchemaFormWidget w;
    QJsonObject values;
    values["type"] = "serial";
    w.setSchema(makeConditionalSchema(), values);

    QComboBox* typeCombo = nullptr;
    const auto combos = w.findChildren<QComboBox*>();
    for (QComboBox* combo : combos)
    {
        if (combo->findData(QStringLiteral("tcp")) >= 0)
        {
            typeCombo = combo;
            break;
        }
    }
    QVERIFY(typeCombo != nullptr);
    if (typeCombo == nullptr)
    {
        return;
    }
    typeCombo->setCurrentIndex(typeCombo->findData(QStringLiteral("tcp")));

    const QJsonObject result = w.values();
    QVERIFY(result.contains("ip"));
    QVERIFY(result.contains("port"));
    QVERIFY(!result.contains("portName"));
    QVERIFY(!result.contains("baudrate"));
}

void TestSchemaFormWidget::valuesOmitsInactiveBranchFields()
{
    SchemaFormWidget w;
    QJsonObject values;
    values["type"] = "tcp";
    w.setSchema(makeConditionalSchema(), values);

    const QJsonObject result = w.values();
    QVERIFY(!result.contains("portName"));
    QVERIFY(!result.contains("baudrate"));
}

void TestSchemaFormWidget::labelHiddenWithWidget()
{
    SchemaFormWidget w;
    QJsonObject values;
    values["type"] = "tcp";
    w.setSchema(makeConditionalSchema(), values);

    // The "Port Name" label must exist and must be hidden when tcp is active
    const auto labels = w.findChildren<QLabel*>();
    bool portNameLabelFound = false;
    for (QLabel* lbl : labels)
    {
        if (lbl->text() == QStringLiteral("Port Name:"))
        {
            portNameLabelFound = true;
            QVERIFY(lbl->isHidden());
        }
    }
    QVERIFY(portNameLabelFound);
}

QTEST_MAIN(TestSchemaFormWidget)
