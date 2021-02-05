#include "serialportcombo.h"

#include <QtSerialPort/QSerialPortInfo>

SerialPortCombo::SerialPortCombo(QWidget *parent)
    : QComboBox(parent)
{

    QComboBox::setEditable(true);
}

void SerialPortCombo::showPopup()
{
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();

    QString currentTxt = QComboBox::currentText();

    QComboBox::clear();

    for(qint32 i = 0; i < list.size(); i++)
    {
        QComboBox::addItem(list.at(i).systemLocation());
    }

    QComboBox::setEditText(currentTxt);

    QComboBox::showPopup();

}
