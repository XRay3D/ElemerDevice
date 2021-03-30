#pragma once

// Qt
#include <QByteArray>
#include <QDebug>
// std
#include <array>
#include <charconv>
#include <concepts>
#include <span>

namespace Elemer {

template <class Cmd>
concept is_command = requires {
    requires std::is_enum_v<Cmd> || std::is_integral_v<Cmd>;
};

template <class T>
concept hex_convertible = requires {
    requires std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_pod_v<T> || std::is_trivial_v<T>;
};

/// добавление убирания разделителя из формируемой посылки
struct SkipSemicolon {
};

/// добавление разделителя в формируемую посылку
struct Semicolon {
};

/// обёртка над std::span для удобства хранения указателя и размена куска из буфера данных
struct Span {
    using span = std::span<char>;
    span data;

    Span(span s) noexcept
        : data{s} {
    }

    Span(char* ptr, size_t size) noexcept
        : data{ptr, size} {
    }

    Span mid(size_t pos, size_t len = std::dynamic_extent) noexcept {
        return data.subspan(pos, len);
    }
    bool startsWith(char c) noexcept {
        return data[0] == c;
    }
    operator QByteArray() const noexcept {
        return {data.data(), static_cast<int>(data.size())};
    }
    size_t size() noexcept {
        return data.size();
    }

    /// преобразование строки в целочисленный тип "Т"
    template <class T>
    int to(bool* ok = nullptr) const noexcept
        requires(std::is_integral_v<std::decay_t<T>> || std::is_enum_v<std::decay_t<T>>) {
        T result{};
        auto [ptr, errCode] = std::from_chars(data.data(), data.data() + data.size(), result /*, 10*/);
        if(ok)
            *ok = (errCode == std::errc());
        return result;
    }

    ///////////////////////////////////////////
    /// преобразование строки в тип "Т" с плавающей точкой
    template <class T>
    int to(bool* ok = nullptr) const noexcept
        requires std::is_floating_point_v<std::decay_t<T>> {
        T result{};
        auto [ptr, errCode] = std::from_chars(data.data(), data.data() + data.size(), result /*, std::chars_format::general*/);
        if(ok)
            *ok = (errCode == std::errc());
        return result;
    }
};

/// обёртка для преобразования значения типа "Т" из в Hex формат
struct ToHex {
    const QByteArray hex;

    template <typename... Ts>
    explicit ToHex(Ts&&... vals)
        : hex((toHex(std::forward<Ts>(vals)) + ...)) {
    }

    operator QByteArray() const noexcept { return hex; }
    size_t size() const noexcept { return hex.size(); }

    template <typename T>
    static auto toHex(T&& val) requires(std::is_arithmetic_v<std::decay_t<T>> || std::is_enum_v<std::decay_t<T>>) {
        return QByteArray(reinterpret_cast<const char*>(&val), sizeof(std::decay_t<T>)).toHex().toUpper();
    }

    template <typename T>
    static auto toHex(T&& val) requires std::is_same_v<std::decay_t<T>, QString> {
        return val.toLocal8Bit().toHex().toUpper();
    }

    template <typename T>
    static auto toHex(T&& val) requires std::is_same_v<std::decay_t<T>, QByteArray> {
        return val.toHex().toUpper();
    }

    template <typename T>
    static auto toHex(T) requires std::is_same_v<std::decay_t<T>, Semicolon> {
        return QByteArray{";"};
    }
};

/// обёртка для получения значения типа "Т" из Hex формата
template <typename T>
struct FromHex {
    T& val;
    using value_type = T;

    operator T&() const noexcept { return val; }
    operator T&() noexcept { return val; }

    auto operator=(const Span& arr) requires(hex_convertible<T>&& std::is_array_v<T>) {
        auto data{QByteArray::fromHex(arr)};
        std::memcpy(val, data.data(), data.size());
        return (*this);
    }

    auto operator=(const Span& arr) requires(hex_convertible<T> && !std::is_array_v<T>) {
        val = *reinterpret_cast<T*>(QByteArray::fromHex(arr).data());
        return (*this);
    }
};
template <typename T>
FromHex(T&) -> FromHex<T>; // template deduction guide

/// обёртка для получения знвчения типа "Т" из строки
template <typename T>
struct FromStr {
    T& val;
    bool ok;
    using value_type = T;

    operator T&() noexcept { return val; }
    operator T&() const noexcept { return val; }

    auto operator=(const Span& arr) requires(std::is_arithmetic_v<T> || std::is_enum_v<T>) {
        val = arr.to<T>(&ok);
        return (*this);
    }

    auto operator=(const Span& arr) requires(std::is_same_v<QByteArray, T> || std::is_same_v<QString, T>) {
        val = arr;
        return (*this);
    }
};
template <typename T>
FromStr(T&) -> FromStr<T>; // template deduction guide

/// Формирование посылкм из данных переданных в конструктор
struct Parcel {
    QByteArray data;

    Parcel(Parcel&&) = default;
    Parcel(const Parcel&) = default;
    Parcel& operator=(Parcel&&) = default;
    Parcel& operator=(const Parcel&) = default;

    template <typename... Ts>
    Parcel(Ts&&... args) {
        data.reserve(255);
        data.append(-1);
        data.append(':');
        (func(std::forward<Ts>(args)), ...);
    }

    operator QByteArray() const { return data; }

    // integral
    template <typename T>
    void func(T arg) requires std::is_integral_v<std::decay_t<T>> {
        std::array<char, 16> str{0};
        auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), arg);
        if(!(ec == std::errc{}))
            qDebug() << static_cast<int>(ec);
        *ptr = ';';
        data.append(str.data());
    }

    // enum
    template <typename T>
    void func(T arg) requires std::is_enum_v<std::decay_t<T>> {
        std::array<char, 16> str{0};
        auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), static_cast<long>(arg));
        if(!(ec == std::errc{}))
            qDebug() << static_cast<int>(ec);
        *ptr = ';';
        data.append(str.data());
    }

    // floating point
    template <typename T>
    void func(T arg) requires std::is_floating_point_v<std::decay_t<T>> {
        std::array<char, 16> str{0};
        auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), arg, std::chars_format::fixed, 5);
        if(!(ec == std::errc{}))
            qDebug() << static_cast<int>(ec);
        *ptr = ';';
        data.append(str.data());
    }

    // ToHex
    template <typename T>
    void func(T&& arg) requires std::is_same_v<std::decay_t<T>, ToHex> {
        assert(arg.size());
        data.append(arg).append(';');
    }

    // QByteArray
    template <typename T>
    void func(T&& arg) requires std::is_same_v<std::decay_t<T>, QByteArray> {
        assert(arg.size());
        data.append(arg).append(';');
    }

    // QString
    template <typename T>
    void func(T&& arg) requires std::is_same_v<std::decay_t<T>, QString> {
        assert(arg.size());
        data.append(arg.toLocal8Bit()).append(';');
    }

    // SkipSemicolon
    template <typename T>
    void func(T) requires std::is_same_v<std::decay_t<T>, SkipSemicolon> {
        data.resize(data.size() - 1);
    }

    // Semicolon
    template <typename T>
    void func(T) requires std::is_same_v<std::decay_t<T>, Semicolon> {
        data.append(';');
    }
};

} // namespace Elemer

Q_DECLARE_METATYPE(Elemer::Parcel)
