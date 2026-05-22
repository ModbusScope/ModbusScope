#ifndef OVERLAYLABEL_H
#define OVERLAYLABEL_H

#include <QObject>
#include <QPointer>

class QLabel;
class QWidget;

class OverlayLabel : public QObject
{
    Q_OBJECT
public:
    explicit OverlayLabel(const QString& text, QWidget* pParent, QObject* parent = nullptr);

    void setVisible(bool visible);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QPointer<QWidget> _pTargetWidget;
    QLabel* _pLabel;
};

#endif // OVERLAYLABEL_H
