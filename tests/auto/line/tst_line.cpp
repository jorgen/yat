#include "../../../backend/line.h"
#include <QtTest/QtTest>

#include <QtQml/QQmlEngine>
#include "../../../backend/screen.h"

class tst_Line: public QObject
{
    Q_OBJECT

private slots:
    void insertStart();
    void insertEdgeOfStyle();
    void insertCompatibleStyle();
    void insertIncompatibleStyle();
    void insertIncompaitibleStylesCrossesBoundary();
    void insertIncomaptibleStylesCrosses22Boundaries();
    void insertIncompatibleStylesCrossesToCompatibleStyle();
};

void tst_Line::insertStart()
{
    QQmlEngine *engine = new QQmlEngine();
    Screen *screen = new Screen(engine);
    screen->setHeight(50);
    screen->setWidth(100);

    Line *line = screen->line_at_cursor();

    screen->setTextStyle(TextStyle::Overlined);
    const char insert_text[] = "This is a test";
    screen->insertAtCursor("This is a test");

    QVector<TextStyleLine> style_list = line->style_list();
    TextStyleLine first_style = style_list.at(0);
    QCOMPARE(first_style.start_index, 0);
    QCOMPARE(first_style.end_index, (int) sizeof insert_text);

}
void tst_Line::insertEdgeOfStyle()
{

}

void tst_Line::insertCompatibleStyle()
{

}

void tst_Line::insertIncompatibleStyle()
{

}

void tst_Line::insertIncompaitibleStylesCrossesBoundary()
{

}

void tst_Line::insertIncomaptibleStylesCrosses22Boundaries()
{

}

void tst_Line::insertIncompatibleStylesCrossesToCompatibleStyle()
{

}


#include <tst_line.moc>
QTEST_MAIN(tst_Line);
