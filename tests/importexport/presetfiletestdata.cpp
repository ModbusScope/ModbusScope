
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

QString PresetFileTestData::cSinglePresetJson = QString(
    "{\n"
    "    \"presets\": [\n"
    "        {\n"
    "            \"name\": \"be-seconds\",\n"
    "            \"fieldseparator\": \";\",\n"
    "            \"decimalseparator\": \",\",\n"
    "            \"thousandseparator\": \" \",\n"
    "            \"commentsequence\": \"//\",\n"
    "            \"column\": 1,\n"
    "            \"labelrow\": 1,\n"
    "            \"datarow\": 2,\n"
    "            \"timeinmilliseconds\": false\n"
    "        }\n"
    "    ]\n"
    "}\n"
);

QString PresetFileTestData::cInvalidFileJson = QString(
    "{\n"
    "    \"presets\": [\n"
    "        {\n"
    "            \"column\": 1\n"
    "        }\n"
    "    ]\n"
    "}\n"
);
