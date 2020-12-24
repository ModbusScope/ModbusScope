#ifndef CONNECTIONFORM_H
#define CONNECTIONFORM_H

#include <QWidget>

/* Forward declaration */
class SettingsModel;

namespace Ui {
class ConnectionForm;
}

class ConnectionForm : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionForm(QWidget *parent = nullptr);
    ~ConnectionForm();

    void fillSettingsModel(SettingsModel* pSettingsModel, quint8 connectionId);

public slots:
    void setState(bool bEnabled);

private slots:
    void connTypeSelected(qint32 index);

private:

    void enableSpecificSettings();

    Ui::ConnectionForm* _pUi;
};

#endif // CONNECTIONFORM_H
