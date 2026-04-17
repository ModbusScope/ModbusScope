#ifndef TST_SCHEMAFORMWIDGET_H
#define TST_SCHEMAFORMWIDGET_H

#include <QObject>

class TestSchemaFormWidget : public QObject
{
    Q_OBJECT

private slots:
    void emptySchemaCreatesEmptyForm();
    void stringPropertyCreatesLineEdit();
    void integerPropertyCreatesSpinBox();
    void numberPropertyCreatesDoubleSpinBox();
    void booleanPropertyCreatesCheckBox();
    void stringEnumCreatesComboBox();
    void integerEnumCreatesComboBox();
    void integerSchemaMinMaxApplied();
    void valuesReflectWidgetChanges();
    void setSchemaResetsOldWidgets();
    void stringEnumWithLabelsShowsLabel();
    void integerEnumWithLabelsShowsLabel();
    void conditionalTcpFieldsVisibleOnLoad();
    void conditionalSerialFieldsVisibleOnLoad();
    void conditionalSwitchShowsSerialHidesTcp();
    void conditionalSwitchShowsTcpHidesSerial();
    void valuesOmitsInactiveBranchFields();
    void labelHiddenWithWidget();
    void integerEnumMissingValueUsesSchemaDefault();
    void stringEnumMissingValueUsesSchemaDefault();
    void integerEnumMissingValueNoSchemaDefaultUsesFirstItem();
};

#endif // TST_SCHEMAFORMWIDGET_H
