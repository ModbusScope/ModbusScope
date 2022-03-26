
#include "projectfiletestdata.h"

QString ProjectFileTestData::cLegacyRegExpressions = QString(
    "<?xml version=\"1.0\"?>                                        \n"\
    "<modbusscope datalevel=\"2\">                                  \n"\
    "  <scope>                                                      \n"\
    "    <register active=\"true\">                                 \n"\
    "       <address>40002</address>                                \n"\
    "       <text>Register 40002</text>                             \n"\
    "       <unsigned>true</unsigned>                               \n"\
    "       <bit32>false</bit32>                                    \n"\
    "       <multiply>3</multiply>                                  \n"\
    "       <divide>2</divide>                                      \n"\
    "       <shift>1</shift>                                        \n"\
    "       <bitmask>0xFF00</bitmask>                               \n"\
    "       <color>#0000ff</color>                                  \n"\
    "       <connectionid>0</connectionid>                          \n"\
    "    </register>                                                \n"\
    "  </scope>                                                     \n"\
    "</modbusscope>                                                 \n"
);

QString ProjectFileTestData::cBothLegacyAndNewRegExpressions = QString(
    "<?xml version=\"1.0\"?>                                        \n"\
    "<modbusscope datalevel=\"2\">                                  \n"\
    "  <scope>                                                      \n"\
    "    <register active=\"true\">                                 \n"\
    "       <address>40002</address>                                \n"\
    "       <text>Register 40002</text>                             \n"\
    "       <unsigned>true</unsigned>                               \n"\
    "       <bit32>false</bit32>                                    \n"\
    "       <multiply>3</multiply>                                  \n"\
    "       <divide>2</divide>                                      \n"\
    "       <shift>1</shift>                                        \n"\
    "       <bitmask>0xFF00</bitmask>                               \n"\
    "       <expression><![CDATA[(VAL+6/2)+5+5+5]]></expression>    \n"\
    "       <color>#0000ff</color>                                  \n"\
    "       <connectionid>0</connectionid>                          \n"\
    "    </register>                                                \n"\
    "  </scope>                                                     \n"\
    "</modbusscope>                                                 \n"
);

QString ProjectFileTestData::cNewRegExpressions = QString(
    "<?xml version=\"1.0\"?>                                        \n"\
    "<modbusscope datalevel=\"2\">                                  \n"\
    "  <scope>                                                      \n"\
    "    <register active=\"true\">                                 \n"\
    "       <address>40002</address>                                \n"\
    "       <text>Register 40002</text>                             \n"\
    "       <unsigned>true</unsigned>                               \n"\
    "       <bit32>false</bit32>                                    \n"\
    "       <expression><![CDATA[(VAL+6/2)+5+5+5]]></expression>    \n"\
    "       <color>#0000ff</color>                                  \n"\
    "       <connectionid>0</connectionid>                          \n"\
    "    </register>                                                \n"\
    "  </scope>                                                     \n"\
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

QString ProjectFileTestData::cConnLegacySingle = QString(
    "<modbusscope datalevel=\"2\">                                   \n"\
    "	<modbus>                                                     \n"\
    "		<connection>                                             \n"\
    "			<ip>127.0.0.2</ip>                                   \n"\
    "			<port>5020</port>                                    \n"\
    "			<slaveid>2</slaveid>                                 \n"\
    "			<timeout>10000</timeout>                             \n"\
    "			<consecutivemax>200</consecutivemax>                 \n"\
    "		</connection>                                            \n"\
    "	</modbus>                                                    \n"\
    "</modbusscope>                                                  \n"
);

QString ProjectFileTestData::cConnLegacyDual = QString(
    "<?xml version=\"1.0\"?>                                          \n"\
    "<modbusscope datalevel=\"2\">                                    \n"\
    " <modbus>                                                      \n"\
    "  <connection>                                                 \n"\
    "   <connectionid>0</connectionid>                              \n"\
    "   <ip>127.0.0.2</ip>                                          \n"\
    "   <port>5020</port>                                            \n"\
    "   <slaveid>2</slaveid>                                        \n"\
    "   <timeout>1001</timeout>                                     \n"\
    "   <consecutivemax>250</consecutivemax>                        \n"\
    "   <int32littleendian>true</int32littleendian>                 \n"\
    "   <persistentconnection>false</persistentconnection>           \n"\
    "  </connection>                                                \n"\
    "  <connection>                                                 \n"\
    "   <connectionid>1</connectionid>                              \n"\
    "   <ip>127.0.0.3</ip>                                          \n"\
    "   <port>5021</port>                                            \n"\
    "   <slaveid>3</slaveid>                                        \n"\
    "   <timeout>1002</timeout>                                     \n"\
    "   <consecutivemax>251</consecutivemax>                        \n"\
    "   <int32littleendian>false</int32littleendian>                 \n"\
    "   <persistentconnection>true</persistentconnection>           \n"\
    "  </connection>                                                \n"\
    " </modbus>                                                     \n"\
    "</modbusscope>                                                 \n"
);

QString ProjectFileTestData::cConnSerial = QString(
    "<?xml version=\"1.0\"?>                                           \n"\
    "<modbusscope datalevel=\"2\">                                     \n"\
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
    "<modbusscope datalevel=\"2\">                                     \n"\
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
    "<modbusscope datalevel=\"2\">                                     \n"\
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
