#include "expressionregex.h"


const QString ExpressionRegex::cOperatorcharacters = R"([\+\-\*\^\/\?\<\>\=\!\%\&\|\~\'\_])";

const QString ExpressionRegex::cNumberDec = R"(\d+)";
const QString ExpressionRegex::cNumberHex = R"(0[x]\d+)";
const QString ExpressionRegex::cNumberBin = R"(0[b]\d+)";

const QString ExpressionRegex::cMatchRegister = R"(\$\{(\d?.*?)\})";
const QString ExpressionRegex::cParseReg = R"(\$\{\s*(\d+)(?:\s*@\s*(\d+))?(?:\s*\:\s*(\w+))?\s*\})";
