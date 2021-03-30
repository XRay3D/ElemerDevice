#pragma once

#include <QMutex>
#include <QSerialPort>
#ifdef EL_LOG
#include <QTime>
#endif

namespace Elemer {

class Device;
struct Parcel;

class Port : public QSerialPort {
    Q_OBJECT
    friend class Device;

signals:
    void message(const QString&, int timout = {});

private:
    Port(Device* kds);
    ~Port();
    void Open(int mode);
    void Close();
    void Write(const Parcel& data);

    void Read();

    QByteArray m_answerData;
    QMutex m_mutex;
    Device* m_asciiDevice;
    int forceReadTimerId {};
#ifdef EL_LOG
    QTime time;
#endif

    // QObject interface
protected:
    void timerEvent(QTimerEvent* event) override;
};

class PortOpener { // RAII
    Device* const pAsciiDevice;

public:
    explicit PortOpener(Device* ad);
    ~PortOpener();
};

}
