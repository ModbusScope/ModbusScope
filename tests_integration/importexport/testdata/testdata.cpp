
#include "testdata.h"

QString TestData::cModbusScopeOldFormat = QString(
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

QString TestData::cModbusScopeNewFormat = QString(
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
    "//Bitmask,0xffff,0xffff"                                       "\n"\
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

QString TestData::cDatasetBe = QString(
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

QString TestData::cDatasetUs = QString(
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


QString TestData::cDatasetColumn2 = QString(
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

QString TestData::cDatasetComment = QString(
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


QString TestData::cDatasetSigned = QString(
    "Time (ms);Register 40001;Register 40002"                      "\n"\
    "48;0;0"                                                      "\n"\
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
    "12059;0;0"                                                      "\n"\
);

QString TestData::cDatasetNoLabel = QString(
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

QString TestData::cDatasetAbsoluteDate = QString(
    "Time (ms);Register 40001;Register 40002;Register 40003"             "\n"\
    "24/07/2019 18:19:31,898;0;6;0"                                      "\n"\
    "24/07/2019 18:19:32,929;1;5;0"                                      "\n"\
    "24/07/2019 18:19:33,957;2;4;0"                                      "\n"\
    "24/07/2019 18:19:34,976;3;3;0"                                      "\n"\
    "24/07/2019 18:19:35,992;4;2;0"                                      "\n"\
    "24/07/2019 18:19:37,006;5;1;0"                                      "\n"\
    "24/07/2019 18:19:38,035;6;0;0"                                      "\n"\
);

QString TestData::cDatasetTimeInSecond = QString(
    "Time (s);Register 40001;Register 40002;Register 40003"        "\n"\
    "0;0;6;0"                                                      "\n"\
    "0,1;1;5;0"                                                      "\n"\
    "0,2;2;4;0"                                                      "\n"\
    "0,3;3;3;0"                                                      "\n"\
    "0,4;4;2;0"                                                      "\n"\
    "0,5;5;1;0"                                                      "\n"\
    "0,5;6;0;0"
);




