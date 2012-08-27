#include "register_qml_types.h"

#include <QtQml>

#include "terminal_item.h"
#include "screen.h"
#include "text.h"
#include "line.h"

void register_qml_types()
{
    qmlRegisterType<TerminalItem>("org.yat", 1, 0, "TerminalItem");
    qmlRegisterType<Screen>();
    qmlRegisterType<Text>();
    qmlRegisterType<Line>();
}
