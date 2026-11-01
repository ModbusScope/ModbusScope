#ifndef COLLAPSIBLEHEADER_H
#define COLLAPSIBLEHEADER_H

#include <QPointer>
#include <QWidget>

class QToolButton;

class CollapsibleHeader : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit CollapsibleHeader(QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    void setContent(QWidget* pContent);

    bool isExpanded() const;
    void setExpanded(bool bExpanded);

    QToolButton* toggleButton() const;

signals:
    void expandedChanged(bool bExpanded);
    void textChanged(const QString& text);

protected:
    void mousePressEvent(QMouseEvent* pEvent) override;

private slots:
    void handleToggled(bool bChecked);

private:
    QToolButton* _pToggle;
    QPointer<QWidget> _pContent;
};

#endif // COLLAPSIBLEHEADER_H
