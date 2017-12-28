#ifndef NOTE_H
#define NOTE_H

#include <QString>

class Note
{
public:
    Note();


    double valueData() const;
    double keyData() const;
    QString text() const;

    void setValueData(double valueData);
    void setKeyData(double keyData);
    void setText(const QString &text);

private:
    double _valueData;
    double _keyData;
    QString _text;

};

#endif // NOTE_H
