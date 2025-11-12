#ifndef ADDABLETABWIDGET_H
#define ADDABLETABWIDGET_H

#include <QTabWidget>
#include <QToolButton>

class AddableTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit AddableTabWidget(QWidget* parent = nullptr);
    ~AddableTabWidget() = default;

    void setTabName(int index, const QString& name);

    void setTabs(QList<QWidget*> pages, const QStringList& names);
    void addNewTab(const QString& name, QWidget* content);

    QWidget* tabContent(int index) const;

signals:
    void tabClosed(int index);
    void addTabRequested(); // Emitted when user clicks the "+" button

public slots:
    void handleCloseTab(int index);

private:
    QToolButton* _addButton = nullptr;
};

#endif // ADDABLETABWIDGET_H