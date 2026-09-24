#include "addabletabwidget.h"

#include <QMessageBox>

AddableTabWidget::AddableTabWidget(QWidget* parent) : QTabWidget(parent)
{
    _addButton = new QToolButton(this);
    _addButton->setText("+");
    _addButton->setCursor(Qt::PointingHandCursor);
    _addButton->setStyleSheet("QToolButton { padding: 2px 6px; }");

    setCornerWidget(_addButton, Qt::TopLeftCorner);
    setTabsClosable(false);

    connect(_addButton, &QToolButton::clicked, this, &AddableTabWidget::addTabRequested);
    connect(this, &QTabWidget::tabCloseRequested, this, &AddableTabWidget::confirmAndCloseTab);
}

//! Asks the user to confirm before removing a tab; kept separate from handleCloseTab() so that
//! slot stays a direct, non-interactive removal (used programmatically, e.g. by tests).
void AddableTabWidget::confirmAndCloseTab(int index)
{
    if (index < 0 || index >= count())
    {
        return;
    }

    const QMessageBox::StandardButton reply = QMessageBox::question(
      this, tr("Remove tab"),
      tr("Are you sure you want to remove \"%1\"?\nAny settings on this tab will be lost.").arg(tabText(index)),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        handleCloseTab(index);
    }
}

void AddableTabWidget::handleCloseTab(int index)
{
    if (count() <= 1)
    {
        return;
    }

    QWidget* page = widget(index);
    removeTab(index);
    emit tabClosed(page);
    if (page)
    {
        page->deleteLater();
    }

    setTabsClosable(count() > 1);
}

void AddableTabWidget::setTabs(const QList<QWidget*>& pages, const QStringList& names)
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
