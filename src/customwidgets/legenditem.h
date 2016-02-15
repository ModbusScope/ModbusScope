#ifndef LEGENDITEM_H
#define LEGENDITEM_H

#include <QLabel>
#include <QHBoxLayout>

class LegendItem : public QFrame
{
    Q_OBJECT
public:
    explicit LegendItem(QWidget *parent = 0);

    void setText(QString txt);
    void setColor(QColor color);

signals:

public slots:

private:


    QHBoxLayout * _pLayout;
    QLabel * _pLabelColor;
    QLabel * _pLabelText;
};

#endif // LEGENDITEM_H
