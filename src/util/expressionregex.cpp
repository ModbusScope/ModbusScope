#include "expressionregex.h"


const QString ExpressionRegex::cOperatorcharacters = "[\\+\\-\\*\\^\\/\\?\\<\\>\\=\\!\\%\\&\\|\\~\\'\\_]";

const QString ExpressionRegex::cNumberDec = "\\d+";
const QString ExpressionRegex::cNumberHex = "0[x]\\d+";
const QString ExpressionRegex::cNumberBin = "0[b]\\d+";

const QString ExpressionRegex::cMatchRegister = "\\$\\{(\\d?.*?)\\}";
const QString ExpressionRegex::cParseReg = "\\$\\{\\s*(\\d+)(?:\\s*@\\s*(\\d+))?(?:\\s*\\:\\s*(\\w+))?\\s*\\}";
