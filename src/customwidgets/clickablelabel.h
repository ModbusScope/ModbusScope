#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR);
    explicit ClickableLabel(QString text, QWidget* parent = Q_NULLPTR);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;

};

#endif // CLICKABLELABEL_H
