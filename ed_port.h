#pragma once

#include <QMutex>
#include <QSerialPort>
#include <chrono>
#include <sstream>
#include <string_view>
//#include <format>

namespace Elemer {

namespace chrono = std::chrono;

struct Timer {

#ifdef __GNUC__
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> t1;
#else
    chrono::time_point<chrono::steady_clock> t1;
#endif
    chrono::duration<double, std::milli> duration;
    //    static inline std::map<const char*, double> avg {};
    //    static inline std::map<const char*, size_t> ctr {};
    std::string_view sv;
    Timer(std::string_view sv = {})
        : t1 { chrono::high_resolution_clock::now() }
        , sv { sv } {
        //        avg = fl ? avg : double {};
        //        ctr = fl ? ctr : int {};
    }

    ~Timer() {
        stop();
        if (sv.data()) {
            if (duration.count() > 1000)
                qDebug("-> %s: %f s", sv.data(), duration.count() / 1000);
            else
                qDebug("-> %s: %f ms", sv.data(), duration.count());
        }
    }

    void start(std::string_view sv_ = {}) { t1 = chrono::high_resolution_clock::now(), sv = sv_; }
    void stop() {
        duration = { chrono::high_resolution_clock::now() - t1 };
        //    avg[string_view.data()] += ms_double.count();
        //    qDebug() << "time (" << string_view.data() << ")" << (avg[string_view.data()] / ++ctr[string_view.data()]) << "us";
    }

    auto str() {
        std::stringstream ss;
        //ss << chrono::hh_mm_ss { chrono::duration_cast<chrono::milliseconds>(t1.time_since_epoch()) }; //std::put_time(t1, "%T");
        return ss.str();
    }
    auto stp() {
        std::stringstream ss;
        //ss << chrono::hh_mm_ss { chrono::duration_cast<chrono::milliseconds>(duration) }; //std::put_time(t1, "%T");
        return ss.str();
    }
};

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
    Device* device;
    int forceReadTimerId {};
#ifdef EL_LOG
    Timer timer;
#endif
    // QObject interface
protected:
    void
    timerEvent(QTimerEvent* event) override;
};

class AlwaysOpen : public std::true_type { // RAII
public:
    explicit AlwaysOpen(Device*) { }
};

class CloseAfterRaad : public std::false_type { // RAII
    Device* const pDevice;

public:
    explicit CloseAfterRaad(Device* ad);
    ~CloseAfterRaad();
};

} // namespace Elemer
