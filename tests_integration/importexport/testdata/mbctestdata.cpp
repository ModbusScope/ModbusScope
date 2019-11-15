
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
                    QList <MbcRegisterData>() << MbcRegisterData(40001, true, "Reg1", 0, false, true)
                                              << MbcRegisterData(40002, true, "Reg2", 0, false, true);


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
                    QList <MbcRegisterData>() << MbcRegisterData(40001, true, "Reg1", 0, false, true)
                                              << MbcRegisterData(40002, true, "Reg2", 1, false, true)
                                              << MbcRegisterData(40003, true, "Reg3", 2, false, true);


