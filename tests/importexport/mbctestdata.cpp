
#include "mbctestdata.h"

QString MbcTestData::cSingleTab = QString(
    "﻿<?xml version=\"1.0\" encoding=\"utf-8\"?>"            "\n"\
    "<modbuscontrol>"                                       "\n"\
    "<tab>"                                                 "\n"\
    "	<name>Tab1</name>"                                  "\n"\
    "	<var><reg>40001</reg><text>Reg1</text><type>uint16</type><rw>r</rw></var>"   "\n"\
    "	<var><reg>40002</reg><text>Reg2</text><type>uint16</type><rw>r</rw></var>"    "\n"\
    "</tab>"                                                "\n"\
    "</modbuscontrol>"                                      "\n"\
);

QStringList MbcTestData::cSingleTab_TabList =
                    QStringList() << "Tab1";

QList <MbcRegisterData> MbcTestData::cSingleTab_RegList =
                    QList <MbcRegisterData>() << MbcRegisterData(40001, ModbusDataType::Type::UNSIGNED_16, "Reg1", 0, true, 0)
                                              << MbcRegisterData(40002, ModbusDataType::Type::UNSIGNED_16, "Reg2", 0, true, 0);


QString MbcTestData::cMultiTab = QString(
    "﻿<?xml version=\"1.0\" encoding=\"utf-8\"?>"        "\n"\
    "<modbuscontrol>"                                   "\n"\
    "<tab>"                                             "\n"\
    "	<name>Tab1</name>"                              "\n"\
    "	<var><reg>40001</reg><text>Reg1</text><type>uint16</type><rw>r</rw></var>"      "\n"\
    "</tab>"                                            "\n"\
    "<tab>"                                             "\n"\
    "	<name>Tab2</name>"                              "\n"\
    "	<var><reg>40002</reg><text>Reg2</text><type>uint16</type><rw>r</rw></var>"    "\n"\
    "</tab>"                                            "\n"\
    "<tab>"                                             "\n"\
    "	<name>Tab3</name>"                              "\n"\
    "   <var><reg>40003</reg><text>Reg3</text><type>uint16</type><rw>r</rw></var>"   "\n"\
    "</tab>"                                            "\n"\
    "</modbuscontrol>"                                  "\n"\
);

QStringList MbcTestData::cMultiTab_TabList =
                    QStringList() << "Tab1"
                                  << "Tab2"
                                  << "Tab3";

QList <MbcRegisterData> MbcTestData::cMultiTab_RegList =
                    QList <MbcRegisterData>() << MbcRegisterData(40001, ModbusDataType::Type::UNSIGNED_16, "Reg1", 0, true, 0)
                                              << MbcRegisterData(40002, ModbusDataType::Type::UNSIGNED_16, "Reg2", 1, true, 0)
                                              << MbcRegisterData(40003, ModbusDataType::Type::UNSIGNED_16, "Reg3", 2, true, 0);

QString MbcTestData::cRegisterOptions = QString(
    "﻿<?xml version=\"1.0\" encoding=\"utf-8\"?>"                                        "\n"\
    "<modbuscontrol>"                                                                   "\n"\
    "<tab>"                                                                             "\n"\
    "	<name>Tab1</name>"                                                              "\n"\
    "	<var><reg>40001</reg><text>Reg1</text><type>uint16</type><rw>r</rw></var>"      "\n"\
    "	<var><reg>40002</reg><text>Reg2</text><type>int16</type><rw>r</rw></var>"       "\n"\
    "	<var><reg>40003</reg><text>Reg3</text><type>uint16</type><rw>w</rw></var>"      "\n"\
    "	<var><reg>40004</reg><text>Reg4</text><type>uint32</type><rw>r</rw></var>"      "\n"\
    "	<var><reg>40006</reg><text>Reg5</text><type>uint16</type><rw>r</rw></var>"      "\n"\
    "	<var><reg>40007</reg><text>Reg6</text><type>uint16</type><decimals>0</decimals><rw>r</rw></var>"      "\n"\
    "	<var><reg>40008</reg><text>Reg7</text><type>uint16</type><decimals>5</decimals><rw>r</rw></var>"      "\n"\
    "</tab>"                                                                            "\n"\
    "</modbuscontrol>"                                                                  "\n"\
);

QStringList MbcTestData::cRegisterOptions_TabList = QStringList() << "Tab1";

QList <MbcRegisterData> MbcTestData::cRegisterOptions_RegList =
                    QList <MbcRegisterData>() << MbcRegisterData(40001, ModbusDataType::Type::UNSIGNED_16, "Reg1", 0, true, 0)
                                              << MbcRegisterData(40002, ModbusDataType::Type::SIGNED_16, "Reg2", 0, true, 0)
                                              << MbcRegisterData(40003, ModbusDataType::Type::UNSIGNED_16, "Reg3", 0, false, 0)
                                              << MbcRegisterData(40004, ModbusDataType::Type::UNSIGNED_32, "Reg4", 0, true, 0)
                                              << MbcRegisterData(40006, ModbusDataType::Type::UNSIGNED_16, "Reg5", 0, true, 0)

                                              << MbcRegisterData(40007, ModbusDataType::Type::UNSIGNED_16, "Reg6", 0, true, 0)
                                              << MbcRegisterData(40008, ModbusDataType::Type::UNSIGNED_16, "Reg7", 0, true, 5)
                                                ;


QString MbcTestData::cAutoincrement = QString(
    "﻿<?xml version=\"1.0\" encoding=\"utf-8\"?>"            "\n"\
    "<modbuscontrol>"                                       "\n"\
    "<tab>"                                                 "\n"\
    "	<name>Tab1</name>"                                  "\n"\
    "	<var><reg>40001</reg><text>Reg1</text><type>uint16</type><rw>r</rw></var>"   "\n"\
    "	<var><reg>*</reg><text>Reg2</text><type>uint16</type><rw>r</rw></var>"    "\n"\
    "	<var><reg>*</reg><text>Reg3</text><type>uint16</type><rw>r</rw></var>"    "\n"\
    "	<var><reg>40010</reg><text>Reg10</text><type>uint16</type><rw>r</rw></var>"   "\n"\
    "	<var><reg>*</reg><text>Reg11</text><type>uint16</type><rw>r</rw></var>"    "\n"\

    "</tab>"                                                "\n"\
    "</modbuscontrol>"                                      "\n"\
);

QStringList MbcTestData::cAutoincrement_TabList =
                    QStringList() << "Tab1";

QList <MbcRegisterData> MbcTestData::cAutoincrement_RegList =
                    QList <MbcRegisterData>() << MbcRegisterData(40001, ModbusDataType::Type::UNSIGNED_16, "Reg1", 0, true, 0)
                                              << MbcRegisterData(40002, ModbusDataType::Type::UNSIGNED_16, "Reg2", 0, true, 0)
                                              << MbcRegisterData(40003, ModbusDataType::Type::UNSIGNED_16, "Reg3", 0, true, 0)
                                              << MbcRegisterData(40010, ModbusDataType::Type::UNSIGNED_16, "Reg10", 0, true, 0)
                                              << MbcRegisterData(40011, ModbusDataType::Type::UNSIGNED_16, "Reg11", 0, true, 0)
                                                 ;
