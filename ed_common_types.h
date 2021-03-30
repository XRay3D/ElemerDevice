#pragma once

#include <QObject>
#include <stdint.h>

namespace Elemer {

#pragma pack(push, 1)

[[maybe_unused]] static constexpr int stdBauds[]{300, 600, 1200, 2400, 4800, 9600, 19200};

enum ProtocolType {
    ASCII,
    ModBus
};

struct DeviceInfo {
    uint16_t Tip;
    const char* const Name;
    uint16_t Timeout;
    uint16_t Channels;
    uint16_t Ustavki;
    ProtocolType Protocol;
    // QString Map;
    // .System : = Set<unsigned char, 0, 255> Commands;
    // TCommandType CmdType[256 /*# range 0..255*/];
    // .System : = Set<unsigned char, 0, 255> Errors;
    // QString ErrStr[256 /*# range 0..255*/];
};

enum DeviceType : uint16_t {
    UnknownDevice = 0, //        Unknown device,

    COMMUTATOR = 16,
    MAN = 50,

    TM_5233 = 1, //             TM 5233,
    TM_5232 = 2, //             TM 5232,
    IRT_1730D_OLD = 3, //       ИРТ 1730D Old,
    IRT_730U = 4, //            ИРТ 1730U,
    IRT_730d = 5, //            ИРТ 1730D,
    TM_231 = 6, //              TM 5231,
    RMT_9D = 7, //              РМТ 49D,
    TM_101 = 8, //              TM 5101,
    IRT_5920 = 9, //            ИРТ 5920(30),
    IRT_5930 = 9, //            ИРТ 5920(30),
    IRTM_2405 = 10, //          ИРТМ-2405,
    PMT_9D = 11, //             PMT 39D,
    USD_01 = 12, //             УСД-01,
    AIR_2 = 13, //              АИР-2,
    IKSU_2000 = 15, //          ИКСУ-2000,

    TM_513X = 16, //            TM 513x,
    TM_5102 = 17, //            TM 5102(3),
    TM_5103 = 17, //            TM 5102(3),
    IRT_1730U_A = 18, //        ИРТ 1730У/А,
    IRT_1730D_A = 19, //        ИРТ 1730D/A,
    RMT_39DA = 20, //           РМТ 39DA,
    IKSU_200 = 21, //           ИКСУ-200,
    RMT_49DA_3 = 22, //         РМТ 49DA/3,
    RMT_49DA_1 = 23, //         РМТ 49DA/1,

    KDS_02 = 25, //             КДС-02,
    TM_5122A = 27, //           TM 5122A,
    IPM_0399_M0 = 28, //        ИПМ 0399/М0,
    IPM_0399_M3 = 30, //        ИПМ 0399/М3,

    RMT_39DEX = 31, //          РМТ 39DEx,
    RMT_49DEX_3 = 32, //        РМТ 49DEx/3,
    RMT_49DEX_1 = 33, //        РМТ 49DEx/1,

    // новые
    ROSA_10 = 34, //            РОСА-10,
    IRT_5501 = 41, //           ИРТ 5501,
    RMT_69 = 42, //             РМТ 69,
    AIR_30 = 43, //             АИР 30,
    RMT_59 = 46, //             РМТ 59,
    BPPS_4090_M1X = 48, //      БППС 4090/М1x,
    BPPS_4090_M12_4 = 49, //    БППС 4090 .М12/4,
    IKSU_260 = 50, //           ИКСУ-260,
    BPPS_4090_Ex = 51, //       БППС 4090 Ex,
    TCM_9610 = 53, //           ТЦМ 9610,
    IRT_5502 = 54, //           ИРТ 5502,
    IRT_5503 = 55, //           ИРТ 5503,
    IRT_5504 = 56, //           ИРТ 5504,
    EL_4019 = 57, //            EL-4019,
    IRT_1730NM = 58, //         ИРТ 1730НМ,
    EL_4015 = 59, //            EL-4015,
    EL_4024 = 60, //            EL-4024,
    IRT_5940 = 65, //           ИРТ 5940,
    RMT_69M = 70, //            РМТ 69M,
    IRT_5922MB = 71, //         ИРТ 5922MB,
    RMT_69L = 72, //            РМТ 69L,
    TM_5102D = 73, //           ТМ 5102Д,
    TM_5103D = 74, //           ТМ 5103Д,
    TM_5104D = 75, //           ТМ 5104Д,
    KP_140E = 76, //            КП-140E,
    RMT_49 = 77, //             РМТ 49,
    DDPN_K = 78, //             ДДПН-К,
    KS_xE = 80, //              КС-xE,
    AKVT_03 = 255, //           АКВТ-03,
};

// clang-format off
[[maybe_unused]]static constexpr DeviceInfo deviceInfo[] = {
    { .Tip = 0,   .Name = "Unknown device",   .Timeout = 1200, .Channels = 0,   .Ustavki = 0,  .Protocol = ASCII }, // 0
    { .Tip = 1,   .Name = "TM 5233",          .Timeout = 400,  .Channels = 8,   .Ustavki = 8,  .Protocol = ASCII }, // 1
    { .Tip = 2,   .Name = "TM 5232",          .Timeout = 400,  .Channels = 4,   .Ustavki = 8,  .Protocol = ASCII }, // 2
    { .Tip = 3,   .Name = "ИРТ 1730D Old",    .Timeout = 400,  .Channels = 1,   .Ustavki = 2,  .Protocol = ASCII }, // 3
    { .Tip = 4,   .Name = "ИРТ 1730U",        .Timeout = 400,  .Channels = 1,   .Ustavki = 2,  .Protocol = ASCII }, // 4
    { .Tip = 5,   .Name = "ИРТ 1730D",        .Timeout = 400,  .Channels = 1,   .Ustavki = 2,  .Protocol = ASCII }, // 5
    { .Tip = 6,   .Name = "TM 5231",          .Timeout = 400,  .Channels = 8,   .Ustavki = 0,  .Protocol = ASCII }, // 6
    { .Tip = 7,   .Name = "РМТ 49D",          .Timeout = 400,  .Channels = 3,   .Ustavki = 12, .Protocol = ASCII }, // 7
    { .Tip = 8,   .Name = "TM 5101",          .Timeout = 400,  .Channels = 6,   .Ustavki = 12, .Protocol = ASCII }, // 8
    { .Tip = 9,   .Name = "ИРТ 5920(30)",     .Timeout = 150,  .Channels = 1,   .Ustavki = 3,  .Protocol = ASCII }, // 9
    { .Tip = 10,  .Name = "ИРТМ-2405",        .Timeout = 400,  .Channels = 128, .Ustavki = 0,  .Protocol = ASCII }, // 10
    { .Tip = 11,  .Name = "PMT 39D",          .Timeout = 400,  .Channels = 6,   .Ustavki = 24, .Protocol = ASCII }, // 11
    { .Tip = 12,  .Name = "УСД-01",           .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 12
    { .Tip = 13,  .Name = "АИР-2",            .Timeout = 1200, .Channels = 3,   .Ustavki = 0,  .Protocol = ASCII }, // 13
    { .Tip = 15,  .Name = "ИКСУ-2000",        .Timeout = 2000, .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 14

    { .Tip = 16,  .Name = "TM 513x",          .Timeout = 400,  .Channels = 8,   .Ustavki = 0,  .Protocol = ASCII }, // 15
    { .Tip = 17,  .Name = "TM 5102(3)",       .Timeout = 400,  .Channels = 8,   .Ustavki = 0,  .Protocol = ASCII }, // 16
    { .Tip = 18,  .Name = "ИРТ 1730У/А",      .Timeout = 400,  .Channels = 1,   .Ustavki = 2,  .Protocol = ASCII }, // 17
    { .Tip = 19,  .Name = "ИРТ 1730D/A",      .Timeout = 400,  .Channels = 1,   .Ustavki = 2,  .Protocol = ASCII }, // 18
    { .Tip = 20,  .Name = "РМТ 39DA",         .Timeout = 400,  .Channels = 6,   .Ustavki = 24, .Protocol = ASCII }, // 19
    { .Tip = 21,  .Name = "ИКСУ-200",         .Timeout = 2000, .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 20
    { .Tip = 22,  .Name = "РМТ 49DA/3",       .Timeout = 400,  .Channels = 3,   .Ustavki = 12, .Protocol = ASCII }, // 21
    { .Tip = 23,  .Name = "РМТ 49DA/1",       .Timeout = 400,  .Channels = 1,   .Ustavki = 4,  .Protocol = ASCII }, // 22

    { .Tip = 25,  .Name = "КДС-02",           .Timeout = 600,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 23
    { .Tip = 27,  .Name = "TM 5122A",         .Timeout = 400,  .Channels = 4,   .Ustavki = 0,  .Protocol = ASCII }, // 24
    { .Tip = 28,  .Name = "ИПМ 0399/М0",      .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 25
    { .Tip = 30,  .Name = "ИПМ 0399/М3",      .Timeout = 1200, .Channels = 1,   .Ustavki = 3,  .Protocol = ASCII }, // 26

    { .Tip = 31,  .Name = "РМТ 39DEx",        .Timeout = 400,  .Channels = 6,   .Ustavki = 24, .Protocol = ASCII }, // 27
    { .Tip = 32,  .Name = "РМТ 49DEx/3",      .Timeout = 400,  .Channels = 3,   .Ustavki = 12, .Protocol = ASCII }, // 28
    { .Tip = 33,  .Name = "РМТ 49DEx/1",      .Timeout = 400,  .Channels = 1,   .Ustavki = 4,  .Protocol = ASCII }, // 29

    // далее - новые приборы
    { .Tip = 34,  .Name = "РОСА-10",          .Timeout = 400,  .Channels = 2,   .Ustavki = 0,  .Protocol = ASCII }, // 30
    { .Tip = 41,  .Name = "ИРТ 5501",         .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 31
    { .Tip = 42,  .Name = "РМТ 69",           .Timeout = 1000, .Channels = 6,   .Ustavki = 0,  .Protocol = ASCII }, // 32
    { .Tip = 43,  .Name = "АИР 30",           .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 33
    { .Tip = 46,  .Name = "РМТ 59",           .Timeout = 1000, .Channels = 6,   .Ustavki = 0,  .Protocol = ASCII }, // 34
    { .Tip = 48,  .Name = "БППС 4090/М1x",    .Timeout = 400,  .Channels = 2,   .Ustavki = 0,  .Protocol = ASCII }, // 35
    { .Tip = 49,  .Name = "БППС 4090 .М12/4", .Timeout = 400,  .Channels = 4,   .Ustavki = 0,  .Protocol = ASCII }, // 36
    { .Tip = 50,  .Name = "ИКСУ-260",         .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 37
    { .Tip = 51,  .Name = "БППС 4090 Ex",     .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 38
    { .Tip = 53,  .Name = "ТЦМ 9610",         .Timeout = 400,  .Channels = 2,   .Ustavki = 0,  .Protocol = ASCII }, // 39
    { .Tip = 54,  .Name = "ИРТ 5502",         .Timeout = 400,  .Channels = 2,   .Ustavki = 0,  .Protocol = ASCII }, // 40
    { .Tip = 55,  .Name = "ИРТ 5503",         .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 41
    { .Tip = 56,  .Name = "ИРТ 5504",         .Timeout = 400,  .Channels = 2,   .Ustavki = 0,  .Protocol = ASCII }, // 42
    { .Tip = 57,  .Name = "EL-4019",          .Timeout = 400,  .Channels = 8,   .Ustavki = 0,  .Protocol = ASCII }, // 43
    { .Tip = 58,  .Name = "ИРТ 1730НМ",       .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 44
    { .Tip = 59,  .Name = "EL-4015",          .Timeout = 400,  .Channels = 6,   .Ustavki = 0,  .Protocol = ASCII }, // 45
    { .Tip = 60,  .Name = "EL-4024",          .Timeout = 400,  .Channels = 4,   .Ustavki = 0,  .Protocol = ASCII }, // 46
    { .Tip = 65,  .Name = "ИРТ 5940",         .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 47
    { .Tip = 70,  .Name = "РМТ 69M",          .Timeout = 1000, .Channels = 6,   .Ustavki = 0,  .Protocol = ASCII }, // 48
    { .Tip = 71,  .Name = "ИРТ 5922MB",       .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ModBus }, // 49
    { .Tip = 72,  .Name = "РМТ 69L",          .Timeout = 1000, .Channels = 6,   .Ustavki = 0,  .Protocol = ASCII }, // 50
    { .Tip = 73,  .Name = "ТМ 5102Д",         .Timeout = 400,  .Channels = 4,   .Ustavki = 0,  .Protocol = ModBus }, // 51
    { .Tip = 74,  .Name = "ТМ 5103Д",         .Timeout = 400,  .Channels = 8,   .Ustavki = 0,  .Protocol = ModBus }, // 52
    { .Tip = 75,  .Name = "ТМ 5104Д",         .Timeout = 400,  .Channels = 16,  .Ustavki = 0,  .Protocol = ModBus }, // 53
    { .Tip = 76,  .Name = "КП-140E",          .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ModBus }, // 54
    { .Tip = 77,  .Name = "РМТ 49",           .Timeout = 1000, .Channels = 3,   .Ustavki = 0,  .Protocol = ASCII }, // 55
    { .Tip = 78,  .Name = "ДДПН-К",           .Timeout = 400,  .Channels = 1,   .Ustavki = 0,  .Protocol = ASCII }, // 56
    { .Tip = 80,  .Name = "КС-xE",            .Timeout = 1000, .Channels = 6,   .Ustavki = 0,  .Protocol = ASCII }, // 57
    { .Tip = 255, .Name = "АКВТ-03",          .Timeout = 400,  .Channels = 3,   .Ustavki = 0,  .Protocol = ModBus } // 255
};
// clang-format on

#pragma pack(pop)

enum Baud : uint8_t {
    Baud300,
    Baud600,
    Baud1200,
    Baud2400,
    Baud4800,
    Baud9600,
    Baud19200,
};

enum class Cmd : uint8_t {
    // clang-format off
    GetDevice       = 0,  // Команда "определение типа прибора"                                     Cmd 0 стр.3
    ReadData        = 1,  // Команда "чтение измеряемой величины"                                   Cmd 1 стр.3

    ProtocolType    = 32, // Команда "Определить разновидность протокола"                           Cmd 32 стр.3
    SetAddress      = 33, // Команда address "Установить адрес прибора (модуля связи)"              Cmd 33 стр.4
    SetBaudRate     = 34, // Команда speed "Установить скорость обмена с прибором (модулем связи)"  Cmd 34 стр.4
    ReadStatus      = 35, // Команда reads "прочитать значение регистра статуса модуля связи"       Cmd 35 стр.4
    ReadNByte       = 36, // Команда readb "прочитать N байт из буфера возврата модуля связи"       Cmd 36 стр.4

    //SeekRetBuf    = 47, // Команда seekb "установка адреса в буфере возврата "                    Cmd 47 стр.4

    GetVer          = 0XFE,
    ResetCpu        = 0XFF
    // clang-format on
};

enum class ParamCmd : uint8_t {
    // clang-format off
    Read    = 37, // Команда readp "Чтение параметра прибора"                               Cmd 37 стр.5
    Write   = 38, // Команда writep "Запись параметра прибора"                              Cmd 38 стр.5
    Modif   = 39, // Команда modifyp "Модификация параметра прибора"                        Cmd 39 стр.5
    // clang-format on
};

//enum class Par : uint16_t {
//    SetPoint = 0x66DA,
//    All = 0xFF00,
//    Enable = 0x65DA,
//};

//enum : uint8_t {
//    Read,
//    Write,
//};

enum class FileCmd : uint8_t {
    // clang-format off
    Open    = 40, // Команда open "открыть файл"                                            Cmd 40 стр.5
    Seek    = 41, // Команда seek "установка адреса в файле"                                Cmd 41 стр.6
    Read    = 42, // Команда read "чтение N байт данных из файла с текущего адреса"         Cmd 42 стр.7
    Write   = 43, // Команда write "запись N байта данных в файл по текущему адресу"        Cmd 43 стр.7
    Close   = 44, // Команда close "закрыть файл"                                           Cmd 44 стр.7
    Tell    = 45, // Команда tell "определить позицию в файле"                              Cmd 45 стр.7
    ChMod   = 46, // Команда chmod " изменить разрешенный доступ для файла"                 Cmd 46 стр.8
    Remove  = 47, // Команда remove "удалить файл"                                          Cmd 47 стр.8
    // clang-format on
};

enum class Seek : uint8_t {
    Set, // 0 (SEEK_SET) – начало буфера возврата, <origin> - начальная позиция.
    Cur, // 1 (SEEK_CUR) – текущая позиция указателя на буфер возврата
    End, // 2 (SEEK_END) – конец буфера возврата
};

enum /*class*/ RetCcode : uint8_t {
    Ok
};

enum class PortPolicy {
    AlwaysOpen,
    CloseAfterRaad,
};

}

//Q_DECLARE_METATYPE(Elemer: = MeasuredValues)
