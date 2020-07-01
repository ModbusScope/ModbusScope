
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
    "       <expression><![CDATA[(REG+6/2)+5+5+5]]></expression>    \n"\
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
    "       <expression><![CDATA[(REG+6/2)+5+5+5]]></expression>    \n"\
    "       <color>#0000ff</color>                                  \n"\
    "       <connectionid>0</connectionid>                          \n"\
    "    </register>                                                \n"\
    "  </scope>                                                     \n"\
    "</modbusscope>                                                 \n"
);
