#pragma once

#include "ed_common_types.h"
#include "ed_port.h"
#include "ed_utils.h"
//my
#include <commoninterfaces.h>
//Qt
#include <QSemaphore>
#include <QThread>
#include <chrono>
#include <concepts>

using namespace std::chrono_literals;

namespace Elemer {

    enum class DTR : bool {
        Off,
        On,
    };

    enum class DTS : bool {
        Off,
        On,
    };

    class Device : public QObject, public CommonInterfaces {
        Q_OBJECT
        friend class Port;
        friend class CloseAfterRaad;

    public:
        using Policy = AlwaysOpen;
        Device(QObject* parent = nullptr, DTR dtr = DTR::Off, DTS dts = DTS::Off);
        ~Device();

        virtual DeviceType type() const = 0;
        bool ping(const QString& portName = {}, int baud = 9600, int addr = 0) override;

        DeviceType getType(int addr);

        bool success();
        bool checkParcel();

        QByteArray calcCrc(const QByteArray& parcel, size_t offset = 0);

        Port* port() const;
        uint8_t address() const;
        bool setAddress(uint8_t address);
        bool setBaudRate(Baud baudRate);

        /// Открытие файла
        bool fileOpen();

        /// Закрытие файла
        bool fileClose();

        /// Установка позиции в файле
        bool fileSeek(uint16_t offset = 0, Seek seek = Seek::Set);

        /// Чтение из файла N байт равных сумме размеров передаваемых типов с последующим преобразованием
        /// из НЕХ формата
        template <typename... Ts>
        bool fileRead(Ts&... vals) {
            Policy(this);
            constexpr size_t packSize = (sizeof(Ts) + ... + 0);
            bool success = isConnected() && readHex<FileCmd::Read, packSize>(vals...);
            return success;
        }

        /// Запись в файл с преобразованием в НЕХ формат
        template <typename... Ts>
        bool fileWrite(Ts&&... data) {
            Policy(this);
            bool success = isConnected() && writeHex<FileCmd::Write>(std::forward<Ts>(data)...) == RetCcode::Ok;
            return success;
        }

        bool fileTell();
        bool fileChMod();
        bool fileRemove();

        /// Запись в устройство с преобразованием в НЕХ формат
        template <auto... Cmds, typename... Ts>
        inline int writeHex(Ts&&... vars) requires(is_command<decltype(Cmds)>&&... && true) {
            Policy(this);
            emit writeParcel(makeParcel(m_address, Cmds..., ToHex { std::forward<Ts>(vars)... }));
            if (wait())
                return m_lastRetCode = m_data[1].startsWith('$') ? m_data[1].mid(1).to<int>() : int{};
            return -1;
        }

        /// Чтение из устройства с преобразованием из НЕХ формата
        template <auto... Cmds, typename... Ret>
        inline bool readHex(Ret&... ret) requires(is_command<decltype(Cmds)>&&... && true) {
            constexpr size_t packSize = (sizeof(Ret) + ... + 0);
            constexpr size_t cmdsSize = sizeof...(Cmds);
            if constexpr (cmdsSize > 0) {
                Policy(this);
                emit writeParcel(makeParcel(m_address, Cmds...));
            }
            if ((cmdsSize == 0 || wait()) && m_data[1].size() == packSize * 2) {
                int ctr {};
                ((FromHex { ret } = m_data[1].mid(ctr, sizeof(Ret) * 2), ctr += sizeof(Ret) * 2), ...);
                return true;
            }
            return false;
        }

        /// Запись в устройство с преобразованием в строчный формат
        template <auto... Cmds, typename... Ts>
        inline int write(Ts&&... vars) requires(is_command<decltype(Cmds)>&&... && true) {
            Policy(this);
            emit writeParcel(makeParcel(m_address, Cmds..., std::forward<Ts>(vars)...));
            if (wait())
                return m_lastRetCode = m_data[1].mid(1).to<int>();
            return -1;
        }

        /// Чтение из устройства с преобразованием из строчного формата
        template <auto... Cmds, typename... Ret>
        inline bool read(Ret&... ret) requires(is_command<decltype(Cmds)>&&... && true) {
            constexpr size_t cmdsSize = sizeof...(Cmds);
            constexpr size_t dataize = sizeof...(Ret);
            if constexpr (cmdsSize > 0) {
                Policy(this);
                emit writeParcel(makeParcel(m_address, Cmds...));
            }
            if ((cmdsSize == 0 || wait()) && dataize && m_data.size() >= (dataize + 2)) {
                int ctr {};
                bool okAll = true;
                ((okAll &= (FromStr { ret } = m_data[1 + ctr++]).ok), ...);
                return okAll;
            }
            return false;
        }

        /// Формирование посылки для отправки в устройство
        template <typename... Ts>
        Parcel& makeParcel(Ts&&... args) {
            parcel = Parcel(std::forward<Ts>(args)...);
            parcel.data.append(calcCrc(parcel.data, 2)).append('\r');
            return parcel;
        }

        /// преобразование из НЕХ формата
        template <typename T>
        auto fromHex(size_t index, bool* ok = nullptr) requires hex_convertible<T> {
            if (index >= m_data.size())
                return T {};
            return fromHex<T>(m_data[index], ok);
        }

        /// преобразование из НЕХ формата
        template <typename T>
        auto fromHex(const QByteArray& data, bool* ok = nullptr) requires hex_convertible<T> {
            auto d = QByteArray::fromHex(data);
            if (d.size() != sizeof(T)) {
                ok ? (*ok = false) : false;
                return T {};
            }
            ok ? (*ok = true) : true;
            return *reinterpret_cast<T*>(const_cast<char*>(d.data()));
        }

        static void waitAllReset() { waitAllSemaphore.acquire(waitAllSemaphore.available()); }
        static bool waitAll(int n, int timeout) { return waitAllSemaphore.tryAcquire(n, timeout); }

    signals:
        void open(int mode) override;
        void close() override;
        void writeParcel(const Elemer::Parcel& data);
        void message(const QString&, int timout = {});

    protected:
        bool wait(int timeout = 3000);

        Port* port_;
        QByteArray rcData_;
        QMutex mutex_;
        QSemaphore semaphore_;
        QThread portThread_;

        DTR dtr;
        DTS dts;

        int m_lastRetCode {};

        static inline QSemaphore waitAllSemaphore;

        std::vector<Span> m_data;

        uint8_t m_address {};

    private:
        Parcel parcel;

        static constexpr uint8_t tableCrc16Lo[] {
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
        };

        static constexpr uint8_t tableCrc16Hi[] {
            0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
            0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
            0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
            0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
            0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
            0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
            0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
            0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
            0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
            0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
            0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
            0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
            0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
            0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
            0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
            0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
        };
    };

} // namespace Elemer
