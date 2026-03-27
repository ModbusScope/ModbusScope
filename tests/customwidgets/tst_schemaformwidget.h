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
};

#endif // TST_SCHEMAFORMWIDGET_H
