#include "expressionregex.h"


const QString ExpressionRegex::cOperatorcharacters = R"([\+\-\*\^\/\?\<\>\=\!\%\&\|\~\'\_])";

const QString ExpressionRegex::cNumberDec = R"(\d+)";
const QString ExpressionRegex::cNumberHex = R"(0[x]\d+)";
const QString ExpressionRegex::cNumberBin = R"(0[b]\d+)";

const QString ExpressionRegex::cMatchRegister = R"(\$\{([ichd]?\d?.*?)\})";
const QString ExpressionRegex::cParseReg = R"(\$\{\s*([ichd]?\d+)(?:\s*@\s*(\d+))?(?:\s*\:\s*(\w+))?\s*\})";
