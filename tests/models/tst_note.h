
#ifndef TEST_NOTE_H__
#define TEST_NOTE_H__

#include <QObject>

class TestNote : public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();

    void defaultConstructor();
    void constructorWithArgs();

    void setPosition_pointF();
    void setPosition_doubles();
    void setText();
    void setDraggable();

private:
};

#endif /* TEST_NOTE_H__ */
