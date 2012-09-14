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

#include "pty_reader.h"

#include <pty.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QSocketNotifier>
#include <QtCore/QDebug>

PtyBuffer::PtyBuffer(QObject *parent)
    : QObject(parent)
    , m_is_available(true)
    , m_size(0)
{

}

void PtyBuffer::setSize(int size)
{
    m_size = size;
}

int PtyBuffer::size() const
{
    return m_size;
}
int PtyBuffer::space() const
{
    return sizeof m_buffer / sizeof *m_buffer;
}

bool PtyBuffer::available() const
{
    return m_is_available;
}

void PtyBuffer::setAvailable(bool available)
{
    m_is_available = available;
}

char *PtyBuffer::buffer()
{
    return m_buffer;
}

void PtyBuffer::release()
{
    emit released(this);
}

static char env_variables[][255] = {
    "TERM=xterm",
    "COLORTERM=xterm",
    "COLORFGBG=15;0",
    "LINES",
    "COLUMNS",
    "TERMCAP"
};
static int env_variables_size = sizeof(env_variables) / sizeof(env_variables[0]);

YatPty::YatPty()
    : m_winsize(0)
    , m_current_buffer(0)
{
    m_event_fd = eventfd(0,EFD_CLOEXEC);
    if (!m_event_fd) {
        qFatal("Failed to create eventfd filedescriptor");
    }

    m_terminal_pid = forkpty(&m_master_fd,
                             NULL,
                             NULL,
                             NULL);

    if (m_terminal_pid == 0) {
        for (int i = 0; i < env_variables_size; i++) {
            ::putenv(env_variables[i]);
        }
        ::execl("/bin/bash", "/bin/bash", (const char *) 0);
    }

    //if you don't have epoll, then convert this to kqueue or figure out how to get the hup event
    //from the socket notifier. Doing an extra poll is not an option
    int epoll_hup = epoll_create1(EPOLL_CLOEXEC);
    epoll_event event;
    event.data.ptr = 0;
    event.events = EPOLLHUP;
    epoll_ctl(epoll_hup, EPOLL_CTL_ADD, m_master_fd, &event);
    QSocketNotifier *hupNotifier = new QSocketNotifier(epoll_hup,QSocketNotifier::Read, this);
    connect(hupNotifier, &QSocketNotifier::activated, this, &YatPty::hangupReceived);

    PtyReader *pty_reader = new PtyReader(this);
    QThread *thread = new QThread(this);
    pty_reader->moveToThread(thread);
    thread->start();
}

YatPty::~YatPty()
{
}

void YatPty::write(const QByteArray &data)
{
    if (::write(m_master_fd, data.constData(), data.size()) < 0) {
        qDebug() << "Something whent wrong when writing to m_master_fd";
    }
}

void YatPty::setWidth(int width, int pixelWidth)
{
    if (!m_winsize) {
        m_winsize = new struct winsize;
        m_winsize->ws_row = 25;
        m_winsize->ws_ypixel = 0;
    }

    m_winsize->ws_col = width;
    m_winsize->ws_xpixel = pixelWidth;
    ioctl(m_master_fd, TIOCSWINSZ, m_winsize);
}

void YatPty::setHeight(int height, int pixelHeight)
{
    if (!m_winsize) {
        m_winsize = new struct winsize;
        m_winsize->ws_col = 80;
        m_winsize->ws_xpixel = 0;
    }
    m_winsize->ws_row = height;
    m_winsize->ws_ypixel = pixelHeight;
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

int YatPty::masterDevice() const
{
    return m_master_fd;
}

int YatPty::eventFd() const
{
    return m_event_fd;
}

void YatPty::queuePtyBuffer(PtyBuffer *buffer)
{
    QMutexLocker locker(&m_buffer_guard);
    m_buffers << buffer;
    writeEventFd();
}

PtyBuffer *YatPty::nextPtyBuffer()
{
    QMutexLocker locker(&m_buffer_guard);
    if (!m_buffers.size() && m_current_buffer)
        readEventFd();
    if (m_current_buffer) {
        m_current_buffer->release();
    }
    if (m_buffers.size()) {
        m_current_buffer = m_buffers.takeFirst();
    } else {
        m_current_buffer = 0;
    }

    return m_current_buffer;
}

void YatPty::socketQuit()
{
    qDebug() << "QUIT";
}

void YatPty::writeEventFd()
{
    uint64_t val = 1;
    size_t written = ::write(m_event_fd, &val, sizeof val);
    if (written != sizeof val)
        qDebug() << "YatPty::writeEventFd failed" << strerror(errno);
}

void YatPty::readEventFd()
{
    uint64_t val;
    size_t read = ::read(m_event_fd, &val, sizeof val);
    if (read != sizeof val)
        qDebug() << "YatPty::readEventFd failed" << strerror(errno);
}
