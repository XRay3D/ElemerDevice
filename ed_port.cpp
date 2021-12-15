#include "ed_port.h"
#include "ed_device.h"
#include "ed_utils.h"

#include <qcoreevent.h>

namespace Elemer {

Port::Port(Device* asciiDevice)
    : m_asciiDevice(asciiDevice)
{
    setBaudRate(Baud9600);
    setParity(NoParity);
    setDataBits(Data8);
    setFlowControl(NoFlowControl);
    connect(this, &QSerialPort::readyRead, this, &Port::Read);
}

Port::~Port()
{
#ifdef FORCE_READ
    killTimer(forceReadTimerId);
#endif
}

void Port::Open(int mode)
{
    if (!open(static_cast<OpenMode>(mode)))
        emit message(errorString());
    m_asciiDevice->m_semaphore.release();
#ifdef FORCE_READ
    if (isOpen())
        forceReadTimerId = startTimer(10ms);
#endif
}

void Port::Close()
{
    close();
    m_asciiDevice->m_semaphore.release();
#ifdef FORCE_READ
    if (forceReadTimerId)
        killTimer(forceReadTimerId), forceReadTimerId = 0;
#endif
}

void Port::Write(const Parcel& data)
{
    if (!isOpen())
        return;
#ifdef EL_LOG
    time.start();
    qDebug() << "    Write" << data << write(data) << time;
#else
    write(data);
#endif
}

void Port::Read()
{
    QMutexLocker locker(&m_mutex);
    m_answerData.append(readAll());
    if (int index = m_answerData.indexOf('\r'); ++index > 0) {
        m_asciiDevice->m_answerData = m_answerData.mid(0, index);
#ifdef EL_LOG
        qDebug() << "    Read" << m_asciiDevice->m_answerData << m_asciiDevice->m_answerData.size() << time.elapsed() << "ms";
#endif
        m_answerData.remove(0, index);
        m_asciiDevice->m_semaphore.release();
    }
}

void Port::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == forceReadTimerId)
        Read();
}

PortOpener::PortOpener(Device* ad)
    : pAsciiDevice(ad)
{
    if (!pAsciiDevice)
        return;
    emit pAsciiDevice->open(QIODevice::ReadWrite);
    pAsciiDevice->connected_ = pAsciiDevice->m_semaphore.tryAcquire(1, 1000); // ждём открытия порта
    if (pAsciiDevice->connected_) {
        pAsciiDevice->m_port->setDataTerminalReady(pAsciiDevice->dtr == DTR::On);
        pAsciiDevice->m_port->setRequestToSend(pAsciiDevice->dts == DTS::On);
        pAsciiDevice->m_portThread.msleep(50);
    }
}

PortOpener::~PortOpener()
{
    if (pAsciiDevice && pAsciiDevice->connected_) {
        emit pAsciiDevice->close();
        pAsciiDevice->m_semaphore.tryAcquire(1, 1000); // ждём закрытия порта
    }
}

}
