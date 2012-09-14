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

#ifndef YAT_PTY_H
#define YAT_PTY_H

#include <unistd.h>
#include <pty.h>

#include <QtCore/QObject>
#include <QtCore/QLinkedList>
#include <QtCore/QMutex>

class QSocketNotifier;
class PtyReadHandler;

class PtyBuffer : public QObject
{
    Q_OBJECT
public:
    PtyBuffer(QObject *parent);

    void setSize(int size);

    int size() const;
    int space() const;

    bool available() const;

    void setAvailable(bool available);

    char *buffer();

    void release();

signals:
    void released(PtyBuffer *buffer);

private:
    bool m_is_available;
    int m_size;
    char m_buffer[64];
};

class YatPty : public QObject
{
    Q_OBJECT
public:
    YatPty();
    ~YatPty();

    void write(const QByteArray &data);

    void setWidth(int width, int pixelWidth = 0);
    void setHeight(int height, int pixelHeight = 0);
    QSize size() const;

    int masterDevice() const;
    int eventFd() const;

    void queuePtyBuffer(PtyBuffer *buffer);
    PtyBuffer *nextPtyBuffer();
signals:
    void hangupReceived();

private:
    void socketQuit();
    void writeEventFd();
    void readEventFd();

    pid_t m_terminal_pid;
    int m_master_fd;
    int m_event_fd;
    char m_slave_file_name[PATH_MAX];
    struct termios m_termios;
    struct winsize *m_winsize;
    QLinkedList<PtyBuffer *>m_buffers;
    PtyBuffer *m_current_buffer;
    QMutex m_buffer_guard;
};

#endif //YAT_PTY_H
