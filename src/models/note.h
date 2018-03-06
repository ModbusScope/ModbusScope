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
    bool draggable() const;

    void setValueData(double valueData);
    void setKeyData(double keyData);
    void setText(const QString &text);
    void setDraggable(bool state);

private:
    double _valueData;
    double _keyData;
    QString _text;
    bool _bDraggable;

};

#endif // NOTE_H
