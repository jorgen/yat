#include "register_qml_types.h"

#include <QtQml/qqml.h>

#include "terminal_item.h"
#include "screen.h"

void register_qml_types()
{
    qmlRegisterType<TerminalItem>("org.yat", 1, 0, "TerminalItem");
    qmlRegisterType<Screen>();
}
