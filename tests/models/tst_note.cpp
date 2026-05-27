
#include "tst_note.h"

#include "models/note.h"

#include <QTest>

void TestNote::init()
{
}

void TestNote::cleanup()
{
}

void TestNote::defaultConstructor()
{
    Note note;

    QCOMPARE(note.position(), QPointF(0.0, 0.0));
    QCOMPARE(note.text(), QStringLiteral(""));
    QCOMPARE(note.draggable(), false);
}

void TestNote::constructorWithArgs()
{
    const QString text = QStringLiteral("Test note");
    const QPointF pos(1.5, 2.5);

    Note note(text, pos);

    QCOMPARE(note.text(), text);
    QCOMPARE(note.position(), pos);
    QCOMPARE(note.draggable(), false);
}

void TestNote::setPosition_pointF()
{
    Note note;
    const QPointF pos(3.0, 7.0);

    note.setPosition(pos);

    QCOMPARE(note.position(), pos);
}

void TestNote::setPosition_doubles()
{
    Note note;

    // key != value to catch a copy-paste error that would pass (key, key)
    note.setPosition(3.0, 7.0);

    QCOMPARE(note.position(), QPointF(3.0, 7.0));
    QCOMPARE(note.position().x(), 3.0);
    QCOMPARE(note.position().y(), 7.0);
}

void TestNote::setText()
{
    Note note;

    QCOMPARE(note.text(), QStringLiteral(""));

    note.setText(QStringLiteral("Hello"));
    QCOMPARE(note.text(), QStringLiteral("Hello"));

    note.setText(QStringLiteral(""));
    QCOMPARE(note.text(), QStringLiteral(""));
}

void TestNote::setDraggable()
{
    Note note;

    QCOMPARE(note.draggable(), false);

    note.setDraggable(true);
    QCOMPARE(note.draggable(), true);

    note.setDraggable(false);
    QCOMPARE(note.draggable(), false);
}

QTEST_GUILESS_MAIN(TestNote)
