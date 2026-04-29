
#include "projectfilejsontestdata.h"

// clang-format off

QString ProjectFileJsonTestData::cTooLowVersion = QString(
    "{"                                         "\n"
    "  \"version\": 5"                          "\n"
    "}"                                         "\n"
);

QString ProjectFileJsonTestData::cTooHighVersion = QString(
    "{"                                         "\n"
    "  \"version\": 99"                         "\n"
    "}"                                         "\n"
);

QString ProjectFileJsonTestData::cMissingVersion = QString(
    "{"                                         "\n"
    "  \"adapters\": []"                        "\n"
    "}"                                         "\n"
);

/* Full file with one modbus adapter, one connection, one device (adapter-level),
   and one generic device entry */
QString ProjectFileJsonTestData::cAdaptersAndDevices = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": ["                                         "\n"
    "    {"                                                     "\n"
    "      \"type\": \"modbus\","                               "\n"
    "      \"settings\": {"                                     "\n"
    "        \"connections\": ["                                 "\n"
    "          {"                                               "\n"
    "            \"id\": 0,"                                    "\n"
    "            \"connectiontype\": \"tcp\","                  "\n"
    "            \"ip\": \"127.0.0.1\","                        "\n"
    "            \"port\": 502,"                                "\n"
    "            \"portname\": \"COM1\","                       "\n"
    "            \"baudrate\": 115200,"                         "\n"
    "            \"parity\": 0,"                                "\n"
    "            \"stopbits\": 1,"                              "\n"
    "            \"databits\": 8,"                              "\n"
    "            \"timeout\": 1000,"                            "\n"
    "            \"persistentconnection\": true,"               "\n"
    "            \"enabled\": true"                             "\n"
    "          }"                                               "\n"
    "        ],"                                                "\n"
    "        \"devices\": ["                                    "\n"
    "          {"                                               "\n"
    "            \"id\": 0,"                                    "\n"
    "            \"connectionid\": 0,"                          "\n"
    "            \"slaveid\": 3,"                               "\n"
    "            \"consecutivemax\": 100,"                      "\n"
    "            \"int32littleendian\": false"                  "\n"
    "          }"                                               "\n"
    "        ]"                                                 "\n"
    "      }"                                                   "\n"
    "    }"                                                     "\n"
    "  ],"                                                      "\n"
    "  \"devices\": ["                                          "\n"
    "    {"                                                     "\n"
    "      \"id\": 1,"                                          "\n"
    "      \"adapterId\": 0,"                                   "\n"
    "      \"name\": \"Device 1\","                             "\n"
    "      \"adapter\": { \"type\": \"modbus\" }"               "\n"
    "    }"                                                     "\n"
    "  ],"                                                      "\n"
    "  \"log\": {"                                              "\n"
    "    \"polltime\": 250,"                                     "\n"
    "    \"absolutetimes\": false,"                             "\n"
    "    \"logtofile\": { \"enabled\": false }"                 "\n"
    "  },"                                                      "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": {"                                             "\n"
    "    \"scale\": {"                                          "\n"
    "      \"xaxis\": { \"mode\": \"auto\" },"                  "\n"
    "      \"yaxis\": ["                                        "\n"
    "        { \"axis\": 0, \"mode\": \"auto\" },"              "\n"
    "        { \"axis\": 1, \"mode\": \"auto\" }"               "\n"
    "      ]"                                                   "\n"
    "    }"                                                     "\n"
    "  }"                                                       "\n"
    "}"                                                         "\n"
);

/* Two adapters, two generic devices */
QString ProjectFileJsonTestData::cMultipleAdaptersAndDevices = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": ["                                         "\n"
    "    {"                                                     "\n"
    "      \"type\": \"modbus\","                               "\n"
    "      \"settings\": { \"connections\": [], \"devices\": [] }" "\n"
    "    },"                                                    "\n"
    "    {"                                                     "\n"
    "      \"type\": \"custom\","                               "\n"
    "      \"settings\": { \"endpoint\": \"tcp://localhost:9999\" }" "\n"
    "    }"                                                     "\n"
    "  ],"                                                      "\n"
    "  \"devices\": ["                                          "\n"
    "    {"                                                     "\n"
    "      \"id\": 1,"                                          "\n"
    "      \"adapterId\": 0,"                                   "\n"
    "      \"name\": \"Modbus Device\","                        "\n"
    "      \"adapter\": { \"type\": \"modbus\" }"               "\n"
    "    },"                                                    "\n"
    "    {"                                                     "\n"
    "      \"id\": 2,"                                          "\n"
    "      \"adapterId\": 1,"                                   "\n"
    "      \"name\": \"Custom Device\","                        "\n"
    "      \"adapter\": { \"type\": \"custom\" }"               "\n"
    "    }"                                                     "\n"
    "  ],"                                                      "\n"
    "  \"log\": { \"polltime\": 500, \"absolutetimes\": true, \"logtofile\": { \"enabled\": false } }," "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": { \"scale\": { \"xaxis\": { \"mode\": \"auto\" }, \"yaxis\": [] } }" "\n"
    "}"                                                         "\n"
);

/* Adapters array only, no generic devices */
QString ProjectFileJsonTestData::cAdapterOnlyNoDevices = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": ["                                         "\n"
    "    {"                                                     "\n"
    "      \"type\": \"modbus\","                               "\n"
    "      \"settings\": { \"connections\": [], \"devices\": [] }" "\n"
    "    }"                                                     "\n"
    "  ],"                                                      "\n"
    "  \"log\": { \"polltime\": 1000, \"absolutetimes\": false, \"logtofile\": { \"enabled\": false } }," "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": { \"scale\": { \"xaxis\": { \"mode\": \"auto\" }, \"yaxis\": [] } }" "\n"
    "}"                                                         "\n"
);

/* Log settings: polltime, absolutetimes, logtofile enabled */
QString ProjectFileJsonTestData::cLogSettings = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": [],"                                       "\n"
    "  \"log\": {"                                              "\n"
    "    \"polltime\": 750,"                                     "\n"
    "    \"absolutetimes\": true,"                              "\n"
    "    \"logtofile\": { \"enabled\": true }"                  "\n"
    "  },"                                                      "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": { \"scale\": { \"xaxis\": { \"mode\": \"auto\" }, \"yaxis\": [] } }" "\n"
    "}"                                                         "\n"
);

/* Log to file with a filename — path is injected at test time via %1 so the
   test data is portable across platforms (use QDir::tempPath() when parsing). */
QString ProjectFileJsonTestData::cLogToFileWithPath = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": [],"                                       "\n"
    "  \"log\": {"                                              "\n"
    "    \"polltime\": 1000,"                                    "\n"
    "    \"absolutetimes\": false,"                             "\n"
    "    \"logtofile\": {"                                       "\n"
    "      \"enabled\": true,"                                   "\n"
    "      \"filename\": \"%1\""                                "\n"
    "    }"                                                     "\n"
    "  },"                                                      "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": { \"scale\": { \"xaxis\": { \"mode\": \"auto\" }, \"yaxis\": [] } }" "\n"
    "}"                                                         "\n"
);

/* Scope with three registers */
QString ProjectFileJsonTestData::cScopeRegisters = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": [],"                                       "\n"
    "  \"log\": { \"polltime\": 1000, \"absolutetimes\": false, \"logtofile\": { \"enabled\": false } }," "\n"
    "  \"scope\": ["                                            "\n"
    "    {"                                                     "\n"
    "      \"active\": true,"                                   "\n"
    "      \"text\": \"Data point\","                           "\n"
    "      \"expression\": \"${40001}/2\","                     "\n"
    "      \"color\": \"#ff0000\","                             "\n"
    "      \"valueaxis\": 0"                                    "\n"
    "    },"                                                    "\n"
    "    {"                                                     "\n"
    "      \"active\": true,"                                   "\n"
    "      \"text\": \"Data point 2\","                         "\n"
    "      \"expression\": \"${40002:s16b}\","                  "\n"
    "      \"color\": \"#0000ff\","                             "\n"
    "      \"valueaxis\": 1"                                    "\n"
    "    },"                                                    "\n"
    "    {"                                                     "\n"
    "      \"active\": false,"                                   "\n"
    "      \"text\": \"Data point 3\","                         "\n"
    "      \"expression\": \"${40003:s16b}*10\","               "\n"
    "      \"color\": \"#00ffff\","                             "\n"
    "      \"valueaxis\": 0"                                    "\n"
    "    }"                                                     "\n"
    "  ],"                                                      "\n"
    "  \"view\": { \"scale\": { \"xaxis\": { \"mode\": \"auto\" }, \"yaxis\": [] } }" "\n"
    "}"                                                         "\n"
);

/* View: x-axis sliding */
QString ProjectFileJsonTestData::cViewScaleSliding = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": [],"                                       "\n"
    "  \"log\": { \"polltime\": 1000, \"absolutetimes\": false, \"logtofile\": { \"enabled\": false } }," "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": {"                                             "\n"
    "    \"scale\": {"                                          "\n"
    "      \"xaxis\": { \"mode\": \"sliding\", \"slidinginterval\": 30 }," "\n"
    "      \"yaxis\": []"                                       "\n"
    "    }"                                                     "\n"
    "  }"                                                       "\n"
    "}"                                                         "\n"
);

/* View: y-axis minmax on axis 0, windowauto on axis 1 */
QString ProjectFileJsonTestData::cViewScaleMinMax = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": [],"                                       "\n"
    "  \"log\": { \"polltime\": 1000, \"absolutetimes\": false, \"logtofile\": { \"enabled\": false } }," "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": {"                                             "\n"
    "    \"scale\": {"                                          "\n"
    "      \"xaxis\": { \"mode\": \"auto\" },"                  "\n"
    "      \"yaxis\": ["                                        "\n"
    "        { \"axis\": 0, \"mode\": \"minmax\", \"min\": -5.5, \"max\": 100.25 }," "\n"
    "        { \"axis\": 1, \"mode\": \"windowauto\" }"         "\n"
    "      ]"                                                   "\n"
    "    }"                                                     "\n"
    "  }"                                                       "\n"
    "}"                                                         "\n"
);

/* View: y-axis windowauto on axis 0 */
QString ProjectFileJsonTestData::cViewScaleWindowAuto = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": [],"                                       "\n"
    "  \"log\": { \"polltime\": 1000, \"absolutetimes\": false, \"logtofile\": { \"enabled\": false } }," "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": {"                                             "\n"
    "    \"scale\": {"                                          "\n"
    "      \"xaxis\": { \"mode\": \"auto\" },"                  "\n"
    "      \"yaxis\": ["                                        "\n"
    "        { \"axis\": 0, \"mode\": \"windowauto\" }"         "\n"
    "      ]"                                                   "\n"
    "    }"                                                     "\n"
    "  }"                                                       "\n"
    "}"                                                         "\n"
);

/* View: all auto */
QString ProjectFileJsonTestData::cViewScaleAuto = QString(
    "{"                                                         "\n"
    "  \"version\": 6,"                                         "\n"
    "  \"adapters\": [],"                                       "\n"
    "  \"log\": { \"polltime\": 1000, \"absolutetimes\": false, \"logtofile\": { \"enabled\": false } }," "\n"
    "  \"scope\": [],"                                          "\n"
    "  \"view\": {"                                             "\n"
    "    \"scale\": {"                                          "\n"
    "      \"xaxis\": { \"mode\": \"auto\" },"                  "\n"
    "      \"yaxis\": ["                                        "\n"
    "        { \"axis\": 0, \"mode\": \"auto\" },"              "\n"
    "        { \"axis\": 1, \"mode\": \"auto\" }"               "\n"
    "      ]"                                                   "\n"
    "    }"                                                     "\n"
    "  }"                                                       "\n"
    "}"                                                         "\n"
);

// clang-format on
