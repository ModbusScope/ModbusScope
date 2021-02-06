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
#ifdef Q_OS_WIN
        QComboBox::addItem(list.at(i).portName());
#else
        QComboBox::addItem(list.at(i).systemLocation());
#endif
    }

    QComboBox::setEditText(currentTxt);

    QComboBox::showPopup();

}
