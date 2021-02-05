#ifndef SERIALPORTCOMBO_H
#define SERIALPORTCOMBO_H

#include <QComboBox>
#include <QObject>

class SerialPortCombo : public QComboBox
{
    Q_OBJECT
public:
    SerialPortCombo(QWidget *parent);


    void showPopup() override;
};

#endif // SERIALPORTCOMBO_H
