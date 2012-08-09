#include "register_qml_types.h"

#include <QtQml>

#include "terminal_item.h"
#include "terminal_screen.h"
#include "text_segment.h"
#include "text_segment_line.h"

void register_qml_types()
{
    qmlRegisterType<TerminalItem>("org.yat", 1, 0, "TerminalItem");
    qmlRegisterType<TerminalScreen>();
    qmlRegisterType<TextSegment>();
    qmlRegisterType<TextSegmentLine>();
}
