#include "../../../backend/line.h"
#include <QtTest/QtTest>

#include <QtQml/QQmlEngine>
#include "../../../backend/screen.h"

class LineHandler
{
public:
    LineHandler() {
        screen.setHeight(50);
        screen.setWidth(100);
        screen.line_at_cursor()->clear();
        QCOMPARE(line()->style_list().size(), 1);
        default_style = line()->style_list().at(0);
        default_text_style = default_style.style;
    }

    Line *line() const
    {
        return screen.line_at_cursor();
    }

    TextStyle default_style;
    TextStyle::Styles default_text_style;
    Screen screen;
};

class tst_Line: public QObject
{
    Q_OBJECT

private slots:
    void insertStart();
    void insertEdgeOfStyle();
    void insertCompatibleStyle();
    void insertIncompatibleStyle();
    void insertIncompaitibleStylesCrossesBoundary();
    void insert3IncompatibleStyles();
    void insertIncomaptibleStylesCrosses2Boundaries();
};

void tst_Line::insertStart()
{
    LineHandler lineHandler;
    Line *line = lineHandler.line();

    QVector<TextStyleLine> old_style_list = line->style_list();
    QCOMPARE(old_style_list.size(), 1);

    QString insert_text("This is a test");
    TextStyle textStyle;
    textStyle.style = TextStyle::Overlined;
    line->insertAtPos(0,insert_text, textStyle);

    QVector<TextStyleLine> new_style_list = line->style_list();
    TextStyleLine first_style = new_style_list.at(0);
    QCOMPARE(first_style.start_index, 0);
    QCOMPARE(first_style.end_index, insert_text.size() - 1);
    QCOMPARE(new_style_list.size(), 2);

}

void tst_Line::insertEdgeOfStyle()
{
    LineHandler lineHandler;
    Line *line = lineHandler.line();

    QString first_text("This is the First");
    TextStyle textStyle;
    textStyle.style = TextStyle::Overlined;
    line->insertAtPos(0,first_text, textStyle);

    QString second_text("This is the Second");
    textStyle.style = TextStyle::Bold;
    line->insertAtPos(first_text.size(), second_text, textStyle);

    QVector<TextStyleLine> style_list = line->style_list();

    QCOMPARE(style_list.size(), 3);

    const TextStyleLine &first_style = style_list.at(0);
    QCOMPARE(first_style.style, TextStyle::Overlined);
    QCOMPARE(first_style.start_index, 0);
    QCOMPARE(first_style.end_index, first_text.size() - 1);

    const TextStyleLine &second_style = style_list.at(1);
    QCOMPARE(second_style.style, TextStyle::Bold);
    QCOMPARE(second_style.start_index, first_text.size());
    QCOMPARE(second_style.end_index, first_text.size()+ second_text.size() - 1);

    const TextStyleLine &third_style = style_list.at(2);
    QCOMPARE(third_style.style, TextStyle::Normal);
    QCOMPARE(third_style.start_index, first_text.size()+ second_text.size());
}

void tst_Line::insertCompatibleStyle()
{
    LineHandler lineHandler;
    Line *line = lineHandler.line();

    QString insert_text("Inserted Text");
    line->insertAtPos(10, insert_text, lineHandler.default_style);

    QVector<TextStyleLine> after_style_list = line->style_list();
    QCOMPARE(after_style_list.size(), 1);
    QCOMPARE(after_style_list.at(0).style, lineHandler.default_text_style);
}

void tst_Line::insertIncompatibleStyle()
{
    LineHandler lineHandler;
    Line *line = lineHandler.line();


    QString insert_text("Inserted Text");
    TextStyle insert_style;
    insert_style.style = TextStyle::Blinking;
    line->insertAtPos(10, insert_text, insert_style);

    QVector<TextStyleLine> after_style_list = line->style_list();
    QCOMPARE(after_style_list.size(), 3);

    const TextStyleLine &first_style = after_style_list.at(0);
    QCOMPARE(first_style.start_index, 0);
    QCOMPARE(first_style.end_index, 9);
    QCOMPARE(first_style.style, lineHandler.default_text_style);

    const TextStyleLine &second_style = after_style_list.at(1);
    QCOMPARE(second_style.start_index, 10);
    QCOMPARE(second_style.end_index, 10 + insert_text.size() -1);
    QCOMPARE(second_style.style, TextStyle::Blinking);

    const TextStyleLine &third_style = after_style_list.at(2);
    QCOMPARE(third_style.start_index, 10 + insert_text.size());
    QCOMPARE(third_style.style, lineHandler.default_text_style);
}

void tst_Line::insertIncompaitibleStylesCrossesBoundary()
{
    LineHandler lineHandler;
    Line *line = lineHandler.line();

    QString insert_text("Inserted Text");
    TextStyle insert_style;
    insert_style.style = TextStyle::Blinking;
    line->insertAtPos(0, insert_text, insert_style);

    QString crosses_boundary("New incompatible text");
    insert_style.style = TextStyle::Framed;
    int insert_pos = insert_text.size()/2;
    line->insertAtPos(insert_pos, crosses_boundary, insert_style);

    QVector<TextStyleLine> after_style_list = line->style_list();
    QCOMPARE(after_style_list.size(), 3);

    const TextStyleLine &first_style = after_style_list.at(0);
    QCOMPARE(first_style.start_index, 0);
    QCOMPARE(first_style.end_index, insert_pos -1);
    QCOMPARE(first_style.style, TextStyle::Blinking);

    const TextStyleLine &second_style = after_style_list.at(1);
    QCOMPARE(second_style.start_index, insert_pos);
    QCOMPARE(second_style.end_index, insert_pos + crosses_boundary.size() -1);
    QCOMPARE(second_style.style, TextStyle::Framed);

    const TextStyleLine &third_style = after_style_list.at(2);
    QCOMPARE(third_style.start_index, insert_pos + crosses_boundary.size());
    QCOMPARE(third_style.style, lineHandler.default_text_style);
}

void tst_Line::insert3IncompatibleStyles()
{
    LineHandler lineHandler;
    Line *line = lineHandler.line();

    QString first_text("First Text");
    TextStyle insert_style;
    insert_style.style = TextStyle::Blinking;
    line->insertAtPos(0, first_text, insert_style);

    QString second_text("Second Text");
    insert_style.style = TextStyle::Italic;
    line->insertAtPos(first_text.size(), second_text, insert_style);

    QString third_text("Third Text");
    insert_style.style = TextStyle::Encircled;
    line->insertAtPos(first_text.size() + second_text.size(), third_text, insert_style);

    QCOMPARE(line->style_list().size(), 4);

    QVector<TextStyleLine> after_style_list = line->style_list();

    const TextStyleLine &first_style = after_style_list.at(0);
    QCOMPARE(first_style.start_index, 0);
    QCOMPARE(first_style.end_index, first_text.size() -1);

    const TextStyleLine &second_style = after_style_list.at(1);
    QCOMPARE(second_style.start_index, first_text.size());
    QCOMPARE(second_style.end_index, first_text.size() + second_text.size() - 1);
    QCOMPARE(second_style.style, TextStyle::Italic);

    const TextStyleLine &third_style = after_style_list.at(2);
    QCOMPARE(third_style.start_index, first_text.size() + second_text.size());
    QCOMPARE(third_style.end_index, first_text.size() + second_text.size() + third_text.size() - 1);
    QCOMPARE(third_style.style, TextStyle::Encircled);

    const TextStyleLine &fourth_style = after_style_list.at(3);
    QCOMPARE(fourth_style.start_index, first_text.size() + second_text.size() + third_text.size());
}
void tst_Line::insertIncomaptibleStylesCrosses2Boundaries()
{
    LineHandler lineHandler;
    Line *line = lineHandler.line();

    QString first_text("First Text");
    TextStyle insert_style;
    insert_style.style = TextStyle::Blinking;
    line->insertAtPos(0, first_text, insert_style);

    QString second_text("Second Text");
    insert_style.style = TextStyle::Italic;
    line->insertAtPos(first_text.size(), second_text, insert_style);

    QString third_text("Third Text");
    insert_style.style = TextStyle::Encircled;
    line->insertAtPos(first_text.size() + second_text.size(), third_text, insert_style);

    QCOMPARE(line->style_list().size(), 4);

    QVector<TextStyleLine> before_style_list = line->style_list();

    QString overlap_first_third;
    overlap_first_third.fill(QChar('A'), second_text.size() + 4);
    insert_style.style = TextStyle::DoubleUnderlined;
    line->insertAtPos(first_text.size() -2, overlap_first_third, insert_style);

    QVector<TextStyleLine> after_style_list = line->style_list();
    QCOMPARE(line->style_list().size(), 4);

    const TextStyleLine &first_style = after_style_list.at(0);
    QCOMPARE(first_style.start_index, 0);
    QCOMPARE(first_style.end_index, first_text.size() - 3);
    QCOMPARE(first_style.style, TextStyle::Blinking);

    const TextStyleLine &second_style = after_style_list.at(1);
    QCOMPARE(second_style.style, TextStyle::DoubleUnderlined);
    QCOMPARE(second_style.start_index, first_text.size() - 2);
    QCOMPARE(second_style.end_index, first_text.size() - 2 + overlap_first_third.size() -1);

    const TextStyleLine &third_style = after_style_list.at(2);
    QCOMPARE(third_style.style, TextStyle::Encircled);
    QCOMPARE(third_style.start_index, first_text.size() - 2 + overlap_first_third.size());
    QCOMPARE(third_style.end_index, first_text.size() - 2 + overlap_first_third.size() + third_text.size() - 1 - 2);

    const TextStyleLine &fourth_style = after_style_list.at(3);
    QCOMPARE(fourth_style.style, lineHandler.default_text_style);
    QCOMPARE(fourth_style.start_index, first_text.size() - 2 + overlap_first_third.size() + third_text.size() - 2);
}

#include <tst_line.moc>
QTEST_MAIN(tst_Line);
