#include "ed_port.h"
#include "ed_device.h"
#include "ed_utils.h"

#include <qcoreevent.h>
#include <ratio>

namespace Elemer {

    Port::Port(Device* asciiDevice)
        : device(asciiDevice) {
        setBaudRate(Baud9600);
        setParity(NoParity);
        setDataBits(Data8);
        setFlowControl(NoFlowControl);
        connect(this, &QSerialPort::readyRead, this, &Port::Read);

        connect(this, &QSerialPort::errorOccurred, [](QSerialPort::SerialPortError error) {
            qWarning() << "SerialPortError" << error;
        });
    }

    Port::~Port() {
#ifdef FORCE_READ
        if (forceReadTimerId)
            killTimer(forceReadTimerId);
#endif
    }

    void Port::Open(int mode) {
        if (!open(static_cast<OpenMode>(mode)))
            emit message(portName() + ": " + errorString());
        device->semaphore_.release();

#ifdef FORCE_READ
        if (isOpen())
            forceReadTimerId = startTimer(10ms);
#endif
    }

    void Port::Close() {
        close();
        device->semaphore_.release();
#ifdef FORCE_READ
        if (forceReadTimerId)
            killTimer(forceReadTimerId), forceReadTimerId = 0;
#endif
    }

    void Port::Write(const Parcel& data) {
//    if (!isOpen())
//        return;
#ifdef EL_LOG
        timer.start();
        write(data);
        qDebug("    Wr %s %s %s", portName().toLocal8Bit().data(), timer.str().data(), data.data.data());
#else
        write(data);
#endif
    }

    void Port::Read() {
        QMutexLocker locker(&m_mutex);
        m_answerData.append(readAll());
        if (int index = m_answerData.indexOf('\r'); ++index > 0) {
            device->rcData_ = m_answerData.mid(0, index);
#ifdef EL_LOG
            timer.stop();
            qDebug("    Rd %s %s %s", portName().toLocal8Bit().data(), timer.stp().data(), device->rcData_.data());
#endif
            m_answerData.remove(0, index);
            device->semaphore_.release();
        }
    }

    void Port::timerEvent(QTimerEvent* event) {
        if (event->timerId() == forceReadTimerId)
            Read();
    }

    CloseAfterRaad::CloseAfterRaad(Device* ad)
        : pDevice(ad) {
        if (!pDevice)
            return;
        emit pDevice->open(QIODevice::ReadWrite);
        pDevice->connected_ = pDevice->semaphore_.tryAcquire(1, 1000); // ждём открытия порта
        if (pDevice->connected_) {
            pDevice->port_->setDataTerminalReady(pDevice->dtr == DTR::On);
            pDevice->port_->setRequestToSend(pDevice->dts == DTS::On);
            pDevice->portThread_.msleep(50);
        }
    }

    CloseAfterRaad::~CloseAfterRaad() {
        if (pDevice && pDevice->connected_) {
            emit pDevice->close();
            pDevice->semaphore_.tryAcquire(1, 1000); // ждём закрытия порта
        }
    }

} // namespace Elemer
