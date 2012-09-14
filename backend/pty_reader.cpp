#include "pty_reader.h"

#include "screen.h"

#include <QtCore/QObject>
#include <QtCore/QSocketNotifier>
#include <QtCore/QThread>

#include <QtCore/QDebug>

PtyReader::PtyReader(YatPty *pty)
    : m_pty(pty)
    , m_buffer_pool_size(1024)
    , m_throtling_guard(m_buffer_pool_size)
{
    connect(m_pty, &QObject::destroyed, this, &QObject::deleteLater);

    QSocketNotifier *notifier = new QSocketNotifier(m_pty->masterDevice(),QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &PtyReader::read);

    for (int i = 0; i < m_buffer_pool_size; i++) {
        PtyBuffer *buffer = new PtyBuffer(this);
        connect(buffer,&PtyBuffer::released, this, &PtyReader::bufferReleased, Qt::DirectConnection);
        m_available_buffers << buffer;
    }
}

void PtyReader::read()
{
    PtyBuffer *buffer = aquireBuffer();
    buffer->setSize(::read(m_pty->masterDevice(),buffer->buffer(),buffer->space()));
    if (!buffer->size()) {
        bufferReleased(buffer);
    }
    m_pty->queuePtyBuffer(buffer);
}

void PtyReader::bufferReleased(PtyBuffer *buffer)
{
    QMutexLocker vector_guard(&m_vector_guard);
    m_available_buffers << buffer;
    m_throtling_guard.release();
}

PtyBuffer *PtyReader::aquireBuffer()
{
    m_throtling_guard.acquire();
    QMutexLocker vector_guard(&m_vector_guard);
    PtyBuffer *to_return = m_available_buffers.at(m_available_buffers.size()-1);
    m_available_buffers.remove(m_available_buffers.size()-1,1);
    return to_return;
}
