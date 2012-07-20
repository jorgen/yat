#include "register_qml_types.h"

#include <QtQml>

#include "terminal_item.h"
#include "terminal_screen.h"
#include "text_segment.h"

void register_qml_types()
{
    qmlRegisterType<TerminalItem>("com.yat", 1, 0, "TerminalItem");
    qmlRegisterType<TerminalState>();
    qmlRegisterType<TerminalScreen>();
    qmlRegisterType<TextSegmentLine>();
    qmlRegisterType<TextSegment>();
}
