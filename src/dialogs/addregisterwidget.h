#ifndef ADDREGISTERWIDGET_H
#define ADDREGISTERWIDGET_H

#include "models/graphdata.h"

#include <QButtonGroup>
#include <QJsonObject>
#include <QWidget>

class SchemaFormWidget;
class SettingsModel;

namespace Ui {
class AddRegisterWidget;
}

class AddRegisterWidget : public QWidget
{
    Q_OBJECT

    friend class TestAddRegisterWidget;

public:
    explicit AddRegisterWidget(SettingsModel* pSettingsModel, const QString& adapterId, QWidget* parent = nullptr);
    ~AddRegisterWidget();

signals:
    void graphDataConfigured(GraphData graphData);

private slots:
    void handleResultAccept();

private:
    void resetFields();
    QString generateExpression();

    Ui::AddRegisterWidget* _pUi;
    SchemaFormWidget* _pAddressForm;
    QJsonObject _addressSchema;
    int _defaultTypeIndex{ 0 };

    SettingsModel* _pSettingsModel;

    QButtonGroup _axisGroup;
};

#endif // ADDREGISTERWIDGET_H
