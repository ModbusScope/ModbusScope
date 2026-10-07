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
    void conditionalWithoutIfRequiredShowsCorrectFields();
    void fieldChangedEmittedOnStringEdit();
    void xRefCreatesComboWithLabelsAndIdData();
    void xRefSelectedIdRoundTripsViaValues();
    void xRefStringValueRoundTrips();
    void optionsFromArrayUsesKeys();
    void optionsFromArrayDefaultsToIdAndName();
    void xRefMissingIdShowsPlaceholderAndIsPreserved();
    void xRefOptionsSetAfterSchemaRepopulateAndKeepSelection();
    void xRefOptionsSetAfterSchemaResolvePlaceholder();
    void xRefWithoutOptionsCreatesEmptyCombo();
    void referenceOptionsForSchemaReadsConfigArrays();
};

#endif // TST_SCHEMAFORMWIDGET_H
