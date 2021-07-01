
#include "presetfiletestdata.h"

QString PresetFileTestData::cSinglePreset = QString(

    "<modbusscope>                                                  \n"\
    "    <parsepreset>                                              \n"\
    "        <name>be-seconds</name>                                \n"\
    "        <fieldseparator><![CDATA[;]]></fieldseparator>         \n"\
    "        <decimalseparator><![CDATA[,]]></decimalseparator>     \n"\
    "        <thousandseparator><![CDATA[ ]]></thousandseparator>   \n"\
    "        <commentSequence><![CDATA[//]]></commentSequence>      \n"\
    "        <column>1</column>                                     \n"\
    "        <labelrow>1</labelrow>                                 \n"\
    "        <datarow>2</datarow>                                   \n"\
    "        <dynamicsession>false</dynamicsession>                 \n"\
    "        <timeinmilliseconds>false</timeinmilliseconds>         \n"\
    "    </parsepreset>                                             \n"\
    "</modbusscope>                                                 \n"\
);


QString PresetFileTestData::cInvalidFile = QString(

    "<modbusscope>                                                  \n"\
    "    <parsepreset>                                              \n"\
    "    </parsepreset>                                             \n"\
    "</modbusscope>                                                 \n"\
);
