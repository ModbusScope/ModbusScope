
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
    "   <persistentconnection>true</persistentconnection>           \n"\
    "  </connection>                                                \n"\
    "  <connection>                                                 \n"\
    "   <connectionid>1</connectionid>                              \n"\
    "   <ip>127.0.0.3</ip>                                          \n"\
    "   <port>5021</port>                                            \n"\
    "   <slaveid>3</slaveid>                                        \n"\
    "   <timeout>1002</timeout>                                     \n"\
    "   <consecutivemax>251</consecutivemax>                        \n"\
    "   <int32littleendian>true</int32littleendian>                 \n"\
    "   <persistentconnection>true</persistentconnection>           \n"\
    "  </connection>                                                \n"\
    " </modbus>                                                     \n"\
    "</modbusscope>                                                 \n"
);
