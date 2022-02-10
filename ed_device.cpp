#include "ed_device.h"

namespace Elemer {

int parcelId = qRegisterMetaType<Parcel>("Parcel");

Device::Device(QObject* parent, DTR dtr, DTS dts)
    : QObject(parent)
    , port_(new Port(this))
    , dtr(dtr)
    , dts(dts) {
    port_->moveToThread(&portThread_);
    connect(&portThread_, &QThread::finished, port_, &QObject::deleteLater);
    connect(this, &Device::open, port_, &Port::Open);
    connect(this, &Device::close, port_, &Port::Close);
    connect(this, &Device::writeParcel, port_, &Port::Write);
    connect(port_, &Port::message, this, &Device::message);
    portThread_.start(QThread::InheritPriority);
}

Device::~Device() {
    Timer t(__FUNCTION__);
    emit close();
    semaphore_.tryAcquire(1, 2000);
    portThread_.quit();
    portThread_.wait();
}

bool Device::ping(const QString& portName, int baud, int addr) {
    QMutexLocker locker(&mutex_);

    connected_ = true;
    semaphore_.acquire(semaphore_.available());
    do {
        emit close();
        if (!semaphore_.tryAcquire(1, 10000)) // ждём закрытия порта
            break;

        if (!portName.isEmpty())
            port_->setPortName(portName);
        if (baud != 0)
            port_->setBaudRate(baud);
#ifdef EL_EMU
        return connected_ = true;
#endif
        if constexpr (Policy::value) {
            emit open(QIODevice::ReadWrite);
            if (!(semaphore_.tryAcquire(1, 2000) && port_->isOpen()))
                break;
            port_->setDataTerminalReady(dtr == DTR::On);
            port_->setRequestToSend(dts == DTS::On);
            portThread_.msleep(50);
        }

        if (getType(addr) != type()) {
            if constexpr (Policy::value)
                emit close();
            break;
        }
        return connected_;
    } while (0);
    return connected_ = false;
}

DeviceType Device::getType(int addr) {
    Timer t(__FUNCTION__);
#ifdef EL_EMU
    return type();
#endif
    Policy(this);
    if (isConnected()) {
        emit writeParcel(makeParcel(addr, Cmd::GetDevice));
        if (wait(1000)) {
            m_address = m_data[0].to<int>();
            return static_cast<DeviceType>(m_data[1].to<int>());
        }
    }
    return {};
}

bool Device::success() {
    if (m_data.size() < 3)
        return false;
    m_lastRetCode = m_data[1].mid(1).to<int>();
    return !m_lastRetCode && m_data[1].startsWith('$');
}

bool Device::checkParcel() {
    if (int index = rcData_.indexOf('!'); index > 0)
        rcData_.remove(0, index);

    if (int index = rcData_.lastIndexOf('\r'); index > 0)
        rcData_.resize(index);

    if (int index = rcData_.lastIndexOf(';') + 1;
        index > 0 && calcCrc(rcData_.left(index), 1).toUInt() == rcData_.right(rcData_.length() - index).toUInt()) {
        m_data.clear();
        index = 0;
        int lastIndex;
        do {
            index = rcData_.indexOf(';', index + 1);
            m_data.empty() ? m_data.emplace_back(rcData_.data() + 1, index - 1)
                           : m_data.emplace_back(rcData_.data() + lastIndex, index - lastIndex);
            lastIndex = index + 1;
        } while (index > -1);
        return true;
    }
    m_data.clear();
    return false;
}

bool Device::wait(int timeout) {
    if (connected_ && semaphore_.tryAcquire(1, timeout)) {
        if (checkParcel())
            return true;
        else
            emit message("Ошибка контрольной суммы.");
    } else {
        connected_ = false;
        emit message("Превышено время ожидания ответа.");
    }
    return {};
}

QByteArray Device::calcCrc(const QByteArray& parcel, size_t offset) {
    // (X^16 + X^15 + X^2 + 1).
    union {
        struct {
            uint8_t lo8;
            uint8_t hi8;
        };
        uint16_t crc16;
    } crc = { .crc16 = 0xFFFF };
    for (auto byte : std::span(parcel.data() + offset, parcel.size() - offset)) {
        uint8_t index = crc.lo8 ^ byte;
        crc.lo8 = crc.hi8 ^ tableCrc16Lo[index];
        crc.hi8 = tableCrc16Hi[index];
    }
    return QByteArray::number(crc.crc16);
}

Port* Device::port() const { return port_; }

uint8_t Device::address() const { return m_address; }

bool Device::setAddress(uint8_t address) {
    Policy(this);
    bool success = isConnected() && write<Cmd::SetAddress>(address) == RetCcode::Ok;
    if (success)
        m_address = address;
    return success;
}

bool Device::setBaudRate(Baud baudRate) {
    Policy(this);
    bool success = isConnected() && write<Cmd::SetBaudRate>(baudRate) == RetCcode::Ok;
    if (success)
        port_->setBaudRate(stdBauds[baudRate]);
    return success;
}

bool Device::fileOpen() {
    Policy(this);
    bool success = isConnected() && write<FileCmd::Open>() == RetCcode::Ok;
    return success;
}

bool Device::fileClose() {
    Policy(this);
    bool success = isConnected() && write<FileCmd::Close>() == RetCcode::Ok;
    return success;
}

bool Device::fileSeek(uint16_t offset, Seek seek) {
    Policy(this);
    bool success = isConnected() && write<FileCmd::Seek>(offset, seek) == RetCcode::Ok;
    return success;
}

////////////////////////////////////////////////////////////
/// \brief PortOpener::PortOpener
/// \param ad
///

} // namespace Elemer
