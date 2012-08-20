/**************************************************************************************************
* Copyright (c) 2012 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
* associated documentation files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute,
* sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
* NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
* OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
***************************************************************************************************/

#include "yat_pty.h"

#include <QtCore/QSocketNotifier>

#include <pty.h>
#include <fcntl.h>

#include <QtCore/QSize>
#include <QtCore/QDebug>

YatPty::YatPty(QObject *parent)
    : QObject(parent)
    , m_buffer_max_size(4096)
    , m_buffer_current_size(0)
    , m_winsize(0)
{
    ::putenv("TERM=xterm");
    m_terminal_pid = forkpty(&m_master_fd,
                             NULL,
                             NULL,
                             NULL);
    if (m_terminal_pid == 0) {
        ::execl("/bin/bash", "/bin/bash", (const char *) 0);
    }
    m_master_fd_read_notify = new QSocketNotifier(m_master_fd, QSocketNotifier::Read);
    connect(m_master_fd_read_notify, &QSocketNotifier::activated,
            this, &YatPty::readyRead);
    m_buffer.resize(m_buffer_max_size);
}

YatPty::~YatPty()
{

}

QByteArray YatPty::read()
{
    char *buf = const_cast<char *>(m_buffer.constData());
    m_buffer_current_size = ::read(m_master_fd,buf, m_buffer_max_size);

    return QByteArray::fromRawData(buf, m_buffer_current_size);
}

void YatPty::write(const QByteArray &data)
{
    ::write(m_master_fd, data.constData(), data.size());
}

void YatPty::setWidth(int width)
{
    if (!m_winsize) {
        m_winsize = new struct winsize;
        m_winsize->ws_row = 25;
        m_winsize->ws_xpixel = 0;
        m_winsize->ws_ypixel = 0;
    }

    m_winsize->ws_col = width;
    ioctl(m_master_fd, TIOCSWINSZ, m_winsize);
}

void YatPty::setHeight(int height)
{
    if (!m_winsize) {
        m_winsize = new struct winsize;
        m_winsize->ws_col = 80;
        m_winsize->ws_xpixel = 0;
        m_winsize->ws_ypixel = 0;
    }
    m_winsize->ws_row = height;
    ioctl(m_master_fd, TIOCSWINSZ, m_winsize);

}

QSize YatPty::size() const
{
    if (!m_winsize) {
        YatPty *that = const_cast<YatPty *>(this);
        that->m_winsize = new struct winsize;
        ioctl(m_master_fd, TIOCGWINSZ, m_winsize);
    }
    return QSize(m_winsize->ws_col, m_winsize->ws_row);
}

bool YatPty::moreInput()
{
    int data_in_buffer;
    ::ioctl(m_master_fd,FIONREAD, &data_in_buffer);
    return data_in_buffer;
}
