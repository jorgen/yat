#ifndef PTYREADER_H
#define PTYREADER_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QSemaphore>
#include <QtCore/QVector>

class PtyBuffer;
class YatPty;


class PtyReader : public QObject
{
    Q_OBJECT
public:
    PtyReader(YatPty *pty);

    void read();

    void setException();

    void bufferReleased(PtyBuffer *buffer);
private:
    PtyBuffer *aquireBuffer();
    YatPty *m_pty;
    QVector<PtyBuffer *> m_available_buffers;
    int m_buffer_pool_size;
    QSemaphore m_throtling_guard;
    QMutex m_vector_guard;
};

#endif // PTYREADER_H
