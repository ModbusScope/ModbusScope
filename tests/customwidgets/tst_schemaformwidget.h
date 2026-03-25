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
};
