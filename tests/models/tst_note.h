
#ifndef TST_NOTE_H
#define TST_NOTE_H

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

#endif /* TST_NOTE_H */
