
#include "presetfiletestdata.h"

QString PresetFileTestData::cSinglePreset = QString(

  "<modbusscope>                                                  \n"
  "    <parsepreset>                                              \n"
  "        <name>be-seconds</name>                                \n"
  "        <fieldseparator><![CDATA[;]]></fieldseparator>         \n"
  "        <decimalseparator><![CDATA[,]]></decimalseparator>     \n"
  "        <thousandseparator><![CDATA[ ]]></thousandseparator>   \n"
  "        <commentSequence><![CDATA[//]]></commentSequence>      \n"
  "        <column>1</column>                                     \n"
  "        <labelrow>1</labelrow>                                 \n"
  "        <datarow>2</datarow>                                   \n"
  "        <dynamicsession>false</dynamicsession>                 \n"
  "        <timeinmilliseconds>false</timeinmilliseconds>         \n"
  "    </parsepreset>                                             \n"
  "</modbusscope>                                                 \n");

QString PresetFileTestData::cInvalidFile = QString(

  "<modbusscope>                                                  \n"
  "    <parsepreset>                                              \n"
  "    </parsepreset>                                             \n"
  "</modbusscope>                                                 \n");

QString PresetFileTestData::cSinglePresetJson = QString("{\n"
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
                                                        "}\n");

QString PresetFileTestData::cInvalidFileJson = QString("{\n"
                                                       "    \"presets\": [\n"
                                                       "        {\n"
                                                       "            \"column\": 1\n"
                                                       "        }\n"
                                                       "    ]\n"
                                                       "}\n");

QString PresetFileTestData::cXmlTabSeparator = QString(
  "<modbusscope>                                                  \n"
  "    <parsepreset>                                              \n"
  "        <name>tab-preset</name>                                \n"
  "        <fieldseparator>\\t</fieldseparator>                   \n"
  "        <decimalseparator>.</decimalseparator>                 \n"
  "    </parsepreset>                                             \n"
  "</modbusscope>                                                 \n");

QString PresetFileTestData::cXmlSkipInvalidLoadValid = QString(
  "<modbusscope>                                                  \n"
  "    <parsepreset>                                              \n"
  "    </parsepreset>                                             \n"
  "    <parsepreset>                                              \n"
  "        <name>valid</name>                                     \n"
  "        <fieldseparator>;</fieldseparator>                     \n"
  "        <decimalseparator>,</decimalseparator>                 \n"
  "    </parsepreset>                                             \n"
  "</modbusscope>                                                 \n");

QString PresetFileTestData::cXmlOptionalFields = QString(
  "<modbusscope>                                                  \n"
  "    <parsepreset>                                              \n"
  "        <name>opt-preset</name>                                \n"
  "        <fieldseparator>;</fieldseparator>                     \n"
  "        <decimalseparator>,</decimalseparator>                 \n"
  "        <labelrow>-1</labelrow>                                \n"
  "        <keyword>test-keyword</keyword>                        \n"
  "        <timeinmilliseconds>true</timeinmilliseconds>          \n"
  "    </parsepreset>                                             \n"
  "</modbusscope>                                                 \n");

QString PresetFileTestData::cXmlMinimalPreset = QString(
  "<modbusscope>                                                  \n"
  "    <parsepreset>                                              \n"
  "        <name>minimal</name>                                   \n"
  "        <fieldseparator>;</fieldseparator>                     \n"
  "        <decimalseparator>,</decimalseparator>                 \n"
  "    </parsepreset>                                             \n"
  "</modbusscope>                                                 \n");

QString PresetFileTestData::cJsonTabSeparator = QString("{\n"
                                                        "    \"presets\": [\n"
                                                        "        {\n"
                                                        "            \"name\": \"tab-preset\",\n"
                                                        "            \"fieldseparator\": \"\\t\",\n"
                                                        "            \"decimalseparator\": \".\"\n"
                                                        "        }\n"
                                                        "    ]\n"
                                                        "}\n");

QString PresetFileTestData::cJsonOptionalFields = QString("{\n"
                                                          "    \"presets\": [\n"
                                                          "        {\n"
                                                          "            \"name\": \"opt-preset\",\n"
                                                          "            \"fieldseparator\": \";\",\n"
                                                          "            \"decimalseparator\": \",\",\n"
                                                          "            \"labelrow\": -1,\n"
                                                          "            \"keyword\": \"test-keyword\",\n"
                                                          "            \"timeinmilliseconds\": true\n"
                                                          "        }\n"
                                                          "    ]\n"
                                                          "}\n");

QString PresetFileTestData::cJsonMinimalPreset = QString("{\n"
                                                         "    \"presets\": [\n"
                                                         "        {\n"
                                                         "            \"name\": \"minimal\",\n"
                                                         "            \"fieldseparator\": \";\",\n"
                                                         "            \"decimalseparator\": \",\"\n"
                                                         "        }\n"
                                                         "    ]\n"
                                                         "}\n");

QString PresetFileTestData::cMixedInvalidEntries = QString("{\n"
                                                           "    \"presets\": [\n"
                                                           "        {\n"
                                                           "            \"column\": 1\n"
                                                           "        },\n"
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
                                                           "}\n");
