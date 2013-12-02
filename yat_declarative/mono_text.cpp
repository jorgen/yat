/******************************************************************************
* Copyright (c) 2012 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#include "mono_text.h"

#include <QtQuick/private/qsgadaptationlayer_p.h>
#include <QtQuick/private/qsgrenderer_p.h>
#include <QtQuick/private/qquickitem_p.h>

MonoText::MonoText(QQuickItem *parent)
    : QQuickItem(parent)
    , m_color_changed(false)
    , m_latin(true)
{
    setFlag(ItemHasContents, true);
}

MonoText::~MonoText()
{

}

QString MonoText::text() const
{
    return m_text;
}

void MonoText::setText(const QString &text)
{
    m_text = text;
    update();
}

QFont MonoText::font() const
{
    return m_font;
}

void MonoText::setFont(const QFont &font)
{
    m_font = font;
    m_raw_font = QRawFont();
    update();
}

QColor MonoText::color() const
{
    return m_color;
}

void MonoText::setColor(const QColor &color)
{
    m_color = color;
    m_color_changed = true;
    update();
}

qreal MonoText::textWidth() const
{
    return m_text_size.width();
}

qreal MonoText::textHeight() const
{
    return m_text_size.height();
}

bool MonoText::latin() const
{
    return m_latin;
}

void MonoText::setLatin(bool latin)
{
    if (latin == m_latin)
        return;

    m_latin = latin;
    emit latinChanged();
}

QSGNode *MonoText::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{
    QSGGlyphNode *node = static_cast<QSGGlyphNode *>(old);
    if (!node) {
        QSGRenderContext *sg = QQuickItemPrivate::get(this)->sceneGraphRenderContext();
        node = sg->sceneGraphContext()->createGlyphNode(sg);
        node->setOwnerElement(this);
        node->setStyle(QQuickText::Normal);
    }

    QGlyphRun glyphrun;
    if (!m_raw_font.isValid())
        m_raw_font = QRawFont::fromFont(m_font, QFontDatabase::Latin);

    glyphrun.setRawFont(m_raw_font);
    glyphrun.setGlyphIndexes(m_raw_font.glyphIndexesForString(m_text));

    QVector<QPointF> positions;
    positions.reserve(m_text.size());

    qreal x_pos = 0;
    qreal max_char_width = m_raw_font.averageCharWidth();
    qreal height = m_raw_font.xHeight();
    qreal ascent = m_raw_font.ascent();
    setImplicitHeight(height);
    setImplicitWidth(max_char_width * qreal(m_text.size()));
    for (int i = 0; i < m_text.size(); i++) {
        positions << QPointF(x_pos, height + ascent);
        x_pos += max_char_width;
    }
    glyphrun.setPositions(positions);

    node->setGlyphs(QPointF(0,height), glyphrun);

    if (m_color_changed) {
        node->setColor(m_color);
        node->setStyleColor(m_color);
        m_color_changed = false;
    }

    node->update();

    return node;
}

