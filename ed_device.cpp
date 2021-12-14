#include "ed_device.h"

namespace Elemer {

int parcelId = qRegisterMetaType<Parcel>("Parcel");

Device::Device(QObject* parent, DTR dtr, DTS dts, PortPolicy policy)
    : QObject(parent)
    , m_port(new Port(this))
    , dtr(dtr)
    , dts(dts)
    , policy(policy)
{
    m_port->moveToThread(&m_portThread);
    connect(&m_portThread, &QThread::finished, m_port, &QObject::deleteLater);
    connect(this, &Device::open, m_port, &Port::Open);
    connect(this, &Device::close, m_port, &Port::Close);
    connect(this, &Device::writeParcel, m_port, &Port::Write);
    connect(m_port, &Port::message, this, &Device::message);
    m_portThread.start(QThread::InheritPriority);
}

Device::~Device()
{
    m_portThread.quit();
    m_portThread.wait();
}

bool Device::ping(const QString& portName, int baud, int addr)
{
    QMutexLocker locker(&m_mutex);
    connected_ = true;
#ifdef EL_EMU
    return connected_;
#endif

    m_semaphore.acquire(m_semaphore.available());
    do {
        emit close();
        if (!m_semaphore.tryAcquire(1, 1000)) // ждём закрытия порта
            break;

        if (!portName.isEmpty())
            m_port->setPortName(portName);
        if (baud != 0)
            m_port->setBaudRate(baud);

        if (policy == PortPolicy::AlwaysOpen) {
            emit open(QIODevice::ReadWrite);
            if (!m_semaphore.tryAcquire(1, 1000) && m_port->isOpen())
                break;
            m_port->setDataTerminalReady(dtr == DTR::On);
            m_port->setRequestToSend(dts == DTS::On);
            m_portThread.msleep(50);
        }

        if (getType(addr) != type()) {
            if (policy == PortPolicy::AlwaysOpen) {
                emit close();
            }
            break;
        }
        return connected_;
    } while (0);
    return connected_ = false;
}

DeviceType Device::getType(int addr)
{
    PortOpener po(policy == PortPolicy::CloseAfterRaad ? this : nullptr);
    if (isConnected()) {
        emit writeParcel(makeParcel(addr, Cmd::GetDevice));
        if (wait(100)) {
            m_address = m_data[0].to<int>();
            return static_cast<DeviceType>(m_data[1].to<int>());
        }
    }
#ifdef EL_EMU
    return type();
#endif
    return {};
}

bool Device::success()
{
    if (m_data.size() < 3)
        return false;
    m_lastRetCode = m_data[1].mid(1).to<int>();
    return !m_lastRetCode && m_data[1].startsWith('$');
}

bool Device::checkParcel()
{
    if (int index = m_answerData.indexOf('!'); index > 0)
        m_answerData.remove(0, index);

    if (int index = m_answerData.lastIndexOf('\r'); index > 0)
        m_answerData.resize(index);

    if (int index = m_answerData.lastIndexOf(';') + 1;
        index > 0 && calcCrc(m_answerData.left(index), 1).toUInt() == m_answerData.right(m_answerData.length() - index).toUInt()) {
        m_data.clear();
        index = 0;
        int lastIndex;
        do {
            index = m_answerData.indexOf(';', index + 1);
            m_data.empty() ? m_data.emplace_back(m_answerData.data() + 1, index - 1)
                           : m_data.emplace_back(m_answerData.data() + lastIndex, index - lastIndex);
            lastIndex = index + 1;
        } while (index > -1);
        return true;
    }
    m_data.clear();
    return false;
}

bool Device::wait(int timeout)
{
    if (connected_ && m_semaphore.tryAcquire(1, timeout)) {
        if (checkParcel())
            return true;
        else
            emit message("Ошибка контрольной суммы");
    } else {
        emit message("Превышено время ожидания");
    }
    return {};
}

QByteArray Device::calcCrc(const QByteArray& parcel, size_t offset)
{
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

Port* Device::port() const { return m_port; }

uint8_t Device::address() const { return m_address; }

bool Device::setAddress(uint8_t address)
{
    PortOpener po(policy == PortPolicy::CloseAfterRaad ? this : nullptr);
    bool success = isConnected() && write<Cmd::SetAddress>(address) == RetCcode::Ok;
    if (success)
        m_address = address;
    return success;
}

bool Device::setBaudRate(Baud baudRate)
{
    PortOpener po(policy == PortPolicy::CloseAfterRaad ? this : nullptr);
    bool success = isConnected() && write<Cmd::SetBaudRate>(baudRate) == RetCcode::Ok;
    if (success)
        m_port->setBaudRate(stdBauds[baudRate]);
    return success;
}

bool Device::fileOpen()
{
    PortOpener po(policy == PortPolicy::CloseAfterRaad ? this : nullptr);
    bool success = isConnected() && write<FileCmd::Open>() == RetCcode::Ok;
    return success;
}

bool Device::fileClose()
{
    PortOpener po(policy == PortPolicy::CloseAfterRaad ? this : nullptr);
    bool success = isConnected() && write<FileCmd::Close>() == RetCcode::Ok;
    return success;
}

bool Device::fileSeek(uint16_t offset, Seek seek)
{
    PortOpener po(policy == PortPolicy::CloseAfterRaad ? this : nullptr);
    bool success = isConnected() && write<FileCmd::Seek>(offset, seek) == RetCcode::Ok;
    return success;
}

////////////////////////////////////////////////////////////
/// \brief PortOpener::PortOpener
/// \param ad
///

}
