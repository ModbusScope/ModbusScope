#ifndef TST_COLLAPSIBLEHEADER_H
#define TST_COLLAPSIBLEHEADER_H

#include <QObject>

class CollapsibleHeader;
class QWidget;

class TestCollapsibleHeader : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void startsCollapsed();
    void expandShowsContentAndTurnsArrow();
    void setContentAppliesCurrentState();
    void clickOnToggleExpands();
    void clickOnRowTogglesSection();

private:
    QWidget* _pParent{ nullptr };
    CollapsibleHeader* _pHeader{ nullptr };
    QWidget* _pContent{ nullptr };
};

#endif // TST_COLLAPSIBLEHEADER_H
