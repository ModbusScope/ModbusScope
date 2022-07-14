
#include "csvdata.h"

QString CsvData::cModbusScopeOldFormat = QString(
    "//ModbusScope version;0.7.4"                                   "\n"\
    "//Start time;24-06-2015 22:24:38"                              "\n"\
    "//End time;24-06-2015 22:24:43"                                "\n"\
    "//Slave IP;127.0.0.1:502"                                      "\n"\
    "//Slave ID;1"                                                  "\n"\
    "//Time-out;1000"                                               "\n"\
    "//Poll interval;1000"                                          "\n"\
    "//Communication success;5"                                     "\n"\
    "//Communication errors;0"                                      "\n"\
    "//"                                                            "\n"\
    ""                                                              "\n"\
    ""                                                              "\n"\
    "Time (ms);Register 40001;Register 40002;Register 40003"        "\n"\
    "25;6;12;18"                                                    "\n"\
    "1024;8;16;24"                                                  "\n"\
    "2025;10;20;30"                                                 "\n"\
    "3025;0;0;0"                                                    "\n"\
    "4024;2;4;6"
);

QString CsvData::cModbusScopeNewFormat = QString(
    "//ModbusScope version,1.6.1"                                   "\n"\
    "//Start time,13-07-2019 21:57:47"                              "\n"\
    "//End time,13-07-2019 21:57:49"                                "\n"\
    "//Slave IP (Connection ID 0),127.0.0.1:502"                    "\n"\
    "//Slave ID (Connection ID 0),1"                                "\n"\
    "//Time-out (Connection ID 0),1000"                             "\n"\
    "//Consecutive max (Connection ID 0),125"                       "\n"\
    "//Poll interval,250"                                           "\n"\
    "//Communication success,0"                                     "\n"\
    "//Communication errors,9"                                      "\n"\
    "//"                                                            "\n"\
    "//Property,Register 40001,Register 40002"                      "\n"\
    "//Register Address,40001,40002"                                "\n"\
    "//Color,#000000,#0000ff"                                       "\n"\
    "//Multiply,1,1 "                                               "\n"\
    "//Divide,1,1"                                                  "\n"\
    "//Bitmask,0xffffffff,0xffffffff"                               "\n"\
    "//Shift,0,0"                                                   "\n"\
    "//ConnectionId,0,0"                                            "\n"\
    "//"                                                            "\n"\
    "//Note,800.605,1.667,\"Test\""                                 "\n"\
    ""                                                              "\n"\
    "//"                                                            "\n"\
    "Time (ms),Register 40001,Register 40002"                       "\n"\
    "37,0,0"                                                        "\n"\
    "262,0,0"                                                       "\n"\
    "513,0,0"                                                       "\n"\
    "763,0,0"                                                       "\n"\
    "1012,0,0"                                                      "\n"\
    "1266,0,0"                                                      "\n"\
    "1517,0,0"                                                      "\n"\
    "1768,0,0"                                                      "\n"\
    "2017,0,0"                                                      "\n"\
);

QString CsvData::cDatasetBe = QString(
    "Time (ms);Register 40001;Register 40002"                       "\n"\
    "48;0;0,5"                                                      "\n"\
    "1049;2;4,5"                                                      "\n"\
    "2049;4;8,5"                                                      "\n"\
    "3058;6;12,5"                                                      "\n"\
    "4059;8;16,5"                                                      "\n"\
    "5058;10;20,5"                                                      "\n"\
    "6059;0;0,5"                                                      "\n"\
    "7058;2;4,5"                                                      "\n"\
    "8059;4;8,5"                                                      "\n"\
    "9059;6;12,5"                                                      "\n"\
    "10058;8;16,5"                                                      "\n"\
    "11058;10;20,5"                                                      "\n"\
    "12059;0;0,5"                                                      "\n"\
);

QString CsvData::cDatasetUs = QString(
    "Time (ms),Register 40001,Register 40002"                        "\n"\
    "48,0,0.5"                                                       "\n"\
    "1049,2,4.5"                                                      "\n"\
    "2049,4,8.5"                                                      "\n"\
    "3058,6,12.5"                                                      "\n"\
    "4059,8,16.5"                                                      "\n"\
    "5058,10,20.5"                                                      "\n"\
    "6059,0,0.5"                                                      "\n"\
    "7058,2,4.5"                                                      "\n"\
    "8059,4,8.5"                                                      "\n"\
    "9059,6,12.5"                                                      "\n"\
    "10058,8,16.5"                                                      "\n"\
    "11058,10,20.5"                                                      "\n"\
    "12059,0,0.5"                                                      "\n"\
);


QString CsvData::cDatasetColumn2 = QString(
    "0;Time (ms);Register 40001;Register 40002"                        "\n"\
    "0;48;0;0"                                                      "\n"\
    "0;1049;2;4"                                                      "\n"\
    "0;2049;4;8"                                                      "\n"\
    "0;3058;6;12"                                                      "\n"\
    "0;4059;8;16"                                                      "\n"\
    "0;5058;10;20"                                                      "\n"\
    "0;6059;0;0"                                                      "\n"\
    "0;7058;2;4"                                                      "\n"\
    "0;8059;4;8"                                                      "\n"\
    "0;9059;6;12"                                                      "\n"\
    "0;10058;8;16"                                                      "\n"\
    "0;11058;10;20"                                                      "\n"\
    "0;12059;0;0"                                                      "\n"\
);

QString CsvData::cDatasetComment = QString(
    "Time (ms);Register 40001;Register 40002"                        "\n"\
    "48;0;0,5"                                                      "\n"\
    "1049;2;4,5"                                                      "\n"\
    "--Comment"                                                      "\n"\
    "2049;4;8,5"                                                      "\n"\
    "3058;6;12,5"                                                      "\n"\
    "4059;8;16,5"                                                      "\n"\
    "5058;10;20,5"                                                      "\n"\
    "6059;0;0,5"                                                      "\n"\
    "7058;2;4,5"                                                      "\n"\
    "--Comment"                                                      "\n"\
    "8059;4;8,5"                                                      "\n"\
    "9059;6;12,5"                                                      "\n"\
    "10058;8;16,5"                                                      "\n"\
    "11058;10;20,5"                                                      "\n"\
    "12059;0;0,5"                                                      "\n"\
    "--Comment"                                                      "\n"\
);


QString CsvData::cDatasetSigned = QString(
    "Time (ms);Register 40001;Register 40002"                      "\n"\
    "48;-2;-2,6"                                                      "\n"\
    "1049;2;-4"                                                      "\n"\
    "2049;4;-8"                                                      "\n"\
    "3058;6;-12"                                                      "\n"\
    "4059;8;-16"                                                      "\n"\
    "5058;10;-20"                                                      "\n"\
    "6059;0;0"                                                      "\n"\
    "7058;-2;4"                                                      "\n"\
    "8059;-4;8"                                                      "\n"\
    "9059;-6;12"                                                      "\n"\
    "10058;-8;16"                                                      "\n"\
    "11058;-10;20"                                                      "\n"\
    "12059;-0,0;-1,5"                                                      "\n"\
);

QString CsvData::cDatasetAbsoluteDate = QString(
    "Time;Register 40001;Register 40002"                               "\n"\
    "24/07/2019 18:19:31,898;0;6"                                      "\n"\
    "24/07/2019 18:19:32,929;1;5"                                      "\n"\
    "24/07/2019 18:19:33,957;2;4"                                      "\n"\
    "24/07/2019 18:19:34,976;3;3"                                      "\n"\
    "24/07/2019 18:19:35,992;4;2"                                      "\n"\
    "24/07/2019 18:19:37,006;5;1"                                      "\n"\
    "24/07/2019 18:19:38,035;6;0"                                      "\n"\
);

QString CsvData::cDatasetTimeInSecond = QString(
    "Time (s);Register 40001;Register 40002"                         "\n"\
    "0;0;6"                                                        "\n"\
    "0,1;1;5"                                                      "\n"\
    "0,2;2;4"                                                      "\n"\
    "0,3;3;3"                                                      "\n"\
    "0,4;4;2"                                                      "\n"\
    "0,5;5;1"                                                      "\n"\
    "0,5;6;0"
);

QString CsvData::cDatasetEmptyLastColumn = QString(
    "Time (ms);Register 40001;"                                    "\n"\
    "10;6;"                                                        "\n"\
    "11;5;"                                                      "\n"\
    "22;4;"                                                      "\n"\
    "33;3;"                                                      "\n"\
    "44;2;"                                                      "\n"\
    "55;1;"                                                      "\n"\
    "56;0;"
);

QString CsvData::cDatasetMultiAxis = QString(
    "//ModbusScope version,3.5.1"                                   "\n"\
    "//Property,Register 40001,Register 40002"                      "\n"\
    "//Axis,0,1,0"                                                  "\n"\
    "Time (ms),Register 40001,Register 40002,Register 40003"        "\n"\
    "5,0,51000,0"
);

QString CsvData::cDatasetExcelChanged = QString(
    "//ModbusScope version;3.5.1;;;;"                                                           "\n"\
    "//Start time;13-07-2022 12:04:07;;;;"                                                      "\n"\
    "//End time;13-07-2022 12:04:17;;;;"                                                        "\n"\
    "//Settings (Connection ID 1);127.0.0.1:502;;;;"                                            "\n"\
    "//Slave ID (Connection ID 1);1;;;;"                                                        "\n"\
    "//Time-out (Connection ID 1);1000;;;;"                                                     "\n"\
    "//Consecutive max (Connection ID 1);125;;;;"                                               "\n"\
    "//Poll interval;250;;;;"                                                                   "\n"\
    "//Communication success;28;;;;"                                                            "\n"\
    "//Communication errors;42;;;;"                                                             "\n"\
    "//;;;;;"                                                                                   "\n"\
    "//Property;Register 1;Register 2;Register 3;Register 4;Register 5"                         "\n"\
    "//Color;#000000;#0000ff;#00ffff;#00ff00;#dcdc00"                                           "\n"\
    "//Expression;${40001};${40002};${40003};${40004};${40005}"                                 "\n"\
    "//Axis;0;0;0;0;0"                                                                          "\n"\
    "//;;;;;"                                                                                   "\n"\
    ";;;;;"                                                                                     "\n"\
    "//;;;;;"                                                                                   "\n"\
    "Time (ms);Register 1;Register 2;Register 3;Register 4;Register 5"                          "\n"\
    "0;0;4;0;0;0"                                                                             "\n"\
    "1;0;4;0;0;0"                                                                            "\n"\
    "2;0;4;0;0;0"                                                                            "\n"\
);


