
#include "projectfiledata.h"

QString ProjectFileData::cDefaultProjectFile = QString(
    "<?xml version=\"1.0\"?>                                        \n"\
    "<modbusscope datalevel=\"2\">                                  \n"\
    "  <modbus>                                                     \n"\
    "    <connection>                                               \n"\
    "       <connectionid>0</connectionid>                          \n"\
    "       <ip>127.0.0.1</ip>                                      \n"\
    "       <port>502</port>                                        \n"\
    "       <slaveid>1</slaveid>                                    \n"\
    "       <timeout>1000</timeout>                                 \n"\
    "       <consecutivemax>125</consecutivemax>                    \n"\
    "       <int32littleendian>true</int32littleendian>             \n"\
    "       <persistentconnection>true</persistentconnection>       \n"\
    "    </connection>                                              \n"\
    "    <connection>                                               \n"\
    "       <connectionid>1</connectionid>                          \n"\
    "       <ip>127.0.0.1</ip>                                      \n"\
    "       <port>502</port>                                        \n"\
    "       <slaveid>1</slaveid>                                    \n"\
    "       <timeout>1000</timeout>                                 \n"\
    "       <consecutivemax>125</consecutivemax>                    \n"\
    "       <int32littleendian>true</int32littleendian>             \n"\
    "       <persistentconnection>true</persistentconnection>       \n"\
    "    </connection>                                              \n"\
    "    <log>                                                      \n"\
    "       <polltime>250</polltime>                                \n"\
    "       <absolutetimes>false</absolutetimes>                    \n"\
    "       <logtofile enabled=\"true\"/>                             \n"\
    "    </log>                                                     \n"\
    "  </modbus>                                                    \n"\
    "  <scope>                                                      \n"\
    "    <register active=\"true\">                                   \n"\
    "       <address>40001</address>                                \n"\
    "       <text>Register 40001</text>                             \n"\
    "       <unsigned>true</unsigned>                               \n"\
    "       <bit32>false</bit32>                                    \n"\
    "       <expression><![CDATA[VAL/2]]></expression>              \n"\
    "       <color>#000000</color>                                  \n"\
    "       <connectionid>0</connectionid>                          \n"\
    "    </register>                                                \n"\
    "    <register active=\"true\">                                   \n"\
    "       <address>40002</address>                                \n"\
    "       <text>Register 40002</text>                             \n"\
    "       <unsigned>true</unsigned>                               \n"\
    "       <bit32>false</bit32>                                    \n"\
    "       <expression><![CDATA[(VAL+6/2)+5+5+5]]></expression>    \n"\
    "       <color>#0000ff</color>                                  \n"\
    "       <connectionid>0</connectionid>                          \n"\
    "    </register>                                                \n"\
    "  </scope>                                                     \n"\
    "  <view>                                                       \n"\
    "    <scale>                                                    \n"\
    "       <xaxis mode=\"auto\"/>                                    \n"\
    "       <yaxis mode=\"auto\"/>                                    \n"\
    "    </scale>                                                   \n"\
    "  </view>                                                      \n"\
    "</modbusscope>                                                 \n"\

);
