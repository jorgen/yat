/*******************************************************************************
* Copyright (c) 2014 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*******************************************************************************/

#include "yat_extension_plugin.h"

#include <QtCore/QByteArray>

#include <QQmlEngine>

#include "terminal_screen.h"
#include "object_destruct_item.h"
#include "screen.h"
#include "text.h"
#include "cursor.h"
#include "mono_text.h"
#include "selection.h"

static const struct {
    const char *type;
    int major, minor;
} qmldir [] = {
    { "Screen", 1, 0},
    { "Text", 1, 0},
    { "Cursor", 1, 0},
    { "Selection", 1, 0},
};

void YatExtensionPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArrayLiteral("Yat"));
    qmlRegisterType<TerminalScreen>("Yat", 1, 0, "TerminalScreen");
    qmlRegisterType<ObjectDestructItem>("Yat", 1, 0, "ObjectDestructItem");
    qmlRegisterType<MonoText>("Yat", 1, 0, "MonoText");
    qmlRegisterType<Screen>();
    qmlRegisterType<Text>();
    qmlRegisterType<Cursor>();
    qmlRegisterType<Selection>();

    const QString filesLocation = baseUrl().toString();
    for (int i = 0; i < int(sizeof(qmldir)/sizeof(qmldir[0])); i++)
        qmlRegisterType(QUrl(filesLocation + "/" + qmldir[i].type + ".qml"), uri, qmldir[i].major, qmldir[i].minor, qmldir[i].type);
}

void YatExtensionPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
    Q_UNUSED(engine);
}
