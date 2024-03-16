
#include "projectfiletestdata.h"

QString ProjectFileTestData::cTooLowDataLevel = QString(
    "<?xml version=\"1.0\"?>                                        \n"\
    "<modbusscope datalevel=\"2\">                                  \n"\
    "</modbusscope>                                                 \n"
);

QString ProjectFileTestData::cTooHighDataLevel = QString(
    "<?xml version=\"1.0\"?>                                        \n"\
    "<modbusscope datalevel=\"99\">                                 \n"\
    "</modbusscope>                                                 \n"
    );

QString ProjectFileTestData::cDataLevel3Expressions = QString(
    "<?xml version=\"1.0\"?>                                                    \n"\
    "<modbusscope datalevel=\"3\">                                              \n"\
    "    <scope>                                                                \n"\
    "        <register active=\"true\">                                         \n"\
    "            <text>Data point</text>                                        \n"\
    "            <expression><![CDATA[${40001}/2]]></expression>                \n"\
    "            <color>#ff0000</color>                                         \n"\
    "        </register>                                                        \n"\
    "        <register active=\"true\">                                         \n"\
    "            <text>Data point 2</text>                                      \n"\
    "            <expression><![CDATA[${40002:s16b}]]></expression>             \n"\
    "            <color>#0000ff</color>                                         \n"\
    "        </register>                                                        \n"\
    "        <register active=\"false\">                                        \n"\
    "            <text>Data point 3</text>                                      \n"\
    "            <expression><![CDATA[${40003@2:s16b}*10]]></expression>        \n"\
    "        </register>                                                        \n"\
    "    </scope>                                                               \n"\
    "</modbusscope>                                                             \n"\
);

QString ProjectFileTestData::cDataLevel4Expressions = QString(
    "<?xml version=\"1.0\"?>                                                    \n"\
    "<modbusscope datalevel=\"4\">                                              \n"\
    "    <scope>                                                                \n"\
    "        <register active=\"true\">                                         \n"\
    "            <text>Data point</text>                                        \n"\
    "            <expression><![CDATA[${40001}/2]]></expression>                \n"\
    "            <color>#ff0000</color>                                         \n"\
    "        </register>                                                        \n"\
    "        <register active=\"true\">                                         \n"\
    "            <text>Data point 2</text>                                      \n"\
    "            <expression><![CDATA[${h10000:s16b}]]></expression>            \n"\
    "            <color>#0000ff</color>                                         \n"\
    "        </register>                                                        \n"\
    "        <register active=\"false\">                                        \n"\
    "            <text>Data point 3</text>                                      \n"\
    "            <expression><![CDATA[${i10000@2}*10]]></expression>            \n"\
    "        </register>                                                        \n"\
    "    </scope>                                                               \n"\
    "</modbusscope>                                                             \n"\
    );

QString ProjectFileTestData::cConnSerial = QString(
    "<?xml version=\"1.0\"?>                                           \n"\
    "<modbusscope datalevel=\"3\">                                     \n"\
    " <modbus>                                                         \n"\
    "  <connection>                                                    \n"\
    "   <enabled>true</enabled>                                        \n"\
    "   <connectionid>0</connectionid>                                 \n"\
    "   <type>serial</type>                                            \n"\
    "   <ip>127.0.0.1</ip>                                             \n"\
    "   <port>502</port>                                               \n"\
    "   <portname>/dev/ttyUSB2</portname>                              \n"\
    "   <baudrate>9600</baudrate>                                      \n"\
    "   <parity>0</parity>                                             \n"\
    "   <stopbits>3</stopbits>                                         \n"\
    "   <databits>7</databits>                                         \n"\
    "   <slaveid>3</slaveid>                                           \n"\
    "   <timeout>1003</timeout>                                        \n"\
    "   <consecutivemax>12</consecutivemax>                            \n"\
    "   <int32littleendian>true</int32littleendian>                    \n"\
    "   <persistentconnection>true</persistentconnection>              \n"\
    "  </connection>                                                   \n"\
    " </modbus>                                                        \n"\
    "</modbusscope>                                                    \n"
);

QString ProjectFileTestData::cConnMixedMulti = QString(
    "<?xml version=\"1.0\"?>                                           \n"\
    "<modbusscope datalevel=\"3\">                                     \n"\
    " <modbus>                                                         \n"\
    "  <connection>                                                    \n"\
    "   <enabled>true</enabled>                                        \n"\
    "   <connectionid>0</connectionid>                                 \n"\
    "   <type>tcp</type>                                               \n"\
    "   <ip>127.0.0.2</ip>                                             \n"\
    "   <port>5020</port>                                              \n"\
    "   <portname>/dev/ttyUSB0</portname>                              \n"\
    "   <baudrate>115200</baudrate>                                    \n"\
    "   <parity>0</parity>                                             \n"\
    "   <stopbits>1</stopbits>                                         \n"\
    "   <databits>8</databits>                                         \n"\
    "   <slaveid>2</slaveid>                                           \n"\
    "   <timeout>1002</timeout>                                        \n"\
    "   <consecutivemax>12</consecutivemax>                            \n"\
    "   <int32littleendian>true</int32littleendian>                    \n"\
    "   <persistentconnection>true</persistentconnection>              \n"\
    "  </connection>                                                   \n"\
    "  <connection>                                                    \n"\
    "   <enabled>false</enabled>                                       \n"\
    "   <connectionid>1</connectionid>                                 \n"\
    "   <type>tcp</type>                                               \n"\
    "   <ip>127.0.0.1</ip>                                             \n"\
    "   <port>502</port>                                               \n"\
    "   <portname>/dev/ttyUSB0</portname>                              \n"\
    "   <baudrate>115200</baudrate>                                    \n"\
    "   <parity>0</parity>                                             \n"\
    "   <stopbits>1</stopbits>                                         \n"\
    "   <databits>8</databits>                                         \n"\
    "   <slaveid>1</slaveid>                                           \n"\
    "   <timeout>1000</timeout>                                        \n"\
    "   <consecutivemax>125</consecutivemax>                           \n"\
    "   <int32littleendian>true</int32littleendian>                    \n"\
    "   <persistentconnection>true</persistentconnection>              \n"\
    "  </connection>                                                   \n"\
    "  <connection>                                                    \n"\
    "   <enabled>true</enabled>                                        \n"\
    "   <connectionid>2</connectionid>                                 \n"\
    "   <type>serial</type>                                            \n"\
    "   <ip>127.0.0.1</ip>                                             \n"\
    "   <port>502</port>                                               \n"\
    "   <portname>/dev/ttyUSB2</portname>                              \n"\
    "   <baudrate>9600</baudrate>                                      \n"\
    "   <parity>0</parity>                                             \n"\
    "   <stopbits>3</stopbits>                                         \n"\
    "   <databits>7</databits>                                         \n"\
    "   <slaveid>3</slaveid>                                           \n"\
    "   <timeout>1003</timeout>                                        \n"\
    "   <consecutivemax>12</consecutivemax>                            \n"\
    "   <int32littleendian>true</int32littleendian>                    \n"\
    "   <persistentconnection>true</persistentconnection>              \n"\
    "  </connection>                                                   \n"\
    " </modbus>                                                        \n"\
    "</modbusscope>                                                    \n"
);

QString ProjectFileTestData::cConnEmpty = QString(
    "<?xml version=\"1.0\"?>                                           \n"\
    "<modbusscope datalevel=\"3\">                                     \n"\
    " <modbus>                                                         \n"\
    "  <connection>                                                    \n"\
    "  </connection>                                                   \n"\
    " </modbus>                                                        \n"\
    "</modbusscope>                                                    \n"
);

QString ProjectFileTestData::cScaleDouble = QString(
    "<?xml version=\"1.0\"?>                                    \n"\
    "<modbusscope datalevel=\"3\">                              \n"\
    "    <view>                                                 \n"\
    "        <scale>                                            \n"\
    "            <xaxis mode=\"sliding\">                       \n"\
    "                <slidinginterval>20</slidinginterval>      \n"\
    "            </xaxis>                                       \n"\
    "            <yaxis mode=\"minmax\">                        \n"\
    "                <min>0</min>                               \n"\
    "                <max>25,5</max>                            \n"\
    "            </yaxis>                                       \n"\
    "        </scale>                                           \n"\
    "    </view>                                                \n"\
    "</modbusscope>                                             \n"
);

QString ProjectFileTestData::cValueAxis = QString(
    "<?xml version=\"1.0\"?>                                                    \n"\
    "<modbusscope datalevel=\"3\">                                              \n"\
    "    <scope>                                                                \n"\
    "        <register active=\"true\">                                         \n"\
    "            <text>Data point</text>                                        \n"\
    "            <expression><![CDATA[${40001}/2]]></expression>                \n"\
    "            <valueaxis>0</valueaxis>                                       \n"\
    "        </register>                                                        \n"\
    "        <register active=\"true\">                                         \n"\
    "            <text>Data point 2</text>                                      \n"\
    "            <expression><![CDATA[${40002:s16b}]]></expression>             \n"\
    "            <valueaxis>1</valueaxis>                                       \n"\
    "        </register>                                                        \n"\
    "        <register active=\"false\">                                        \n"\
    "            <text>Data point 3</text>                                      \n"\
    "            <expression><![CDATA[${40003@2:s16b}*10]]></expression>        \n"\
    "        </register>                                                        \n"\
    "    </scope>                                                               \n"\
    "</modbusscope>                                                             \n"\
);
