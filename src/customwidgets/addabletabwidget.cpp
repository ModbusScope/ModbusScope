#include "addabletabwidget.h"

AddableTabWidget::AddableTabWidget(QWidget* parent) : QTabWidget(parent)
{
    _addButton = new QToolButton(this);
    _addButton->setText("+");
    _addButton->setCursor(Qt::PointingHandCursor);
    _addButton->setStyleSheet("QToolButton { padding: 2px 6px; }");

    setCornerWidget(_addButton, Qt::TopLeftCorner);
    setTabsClosable(false);

    connect(_addButton, &QToolButton::clicked, this, &AddableTabWidget::addTabRequested);
    connect(this, &QTabWidget::tabCloseRequested, this, &AddableTabWidget::handleCloseTab);
}

void AddableTabWidget::handleCloseTab(int index)
{
    if (count() <= 1)
    {
        return;
    }

    emit tabClosed(index);

    QWidget* page = widget(index);
    removeTab(index);
    delete page;

    setTabsClosable(count() > 1);
}

void AddableTabWidget::setTabs(QList<QWidget*> pages, const QStringList& names)
{
    if (pages.isEmpty() || names.isEmpty() || pages.size() != names.size())
    {
        return;
    }

    clear();

    for (int i = 0; i < pages.size(); ++i)
    {
        addTab(pages[i], names[i]);
    }

    setTabsClosable(count() > 1);
}

void AddableTabWidget::addNewTab(const QString& name, QWidget* content)
{
    addTab(content, name);
    setTabsClosable(count() > 1);
}

QWidget* AddableTabWidget::tabContent(int index) const
{
    if (index >= 0 && index < count())
    {
        return widget(index);
    }
    return nullptr;
}

void AddableTabWidget::setTabName(int index, const QString& name)
{
    if (index >= 0 && index < count())
    {
        setTabText(index, name);
    }
}
