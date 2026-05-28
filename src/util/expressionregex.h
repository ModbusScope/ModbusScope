#ifndef EXPRESSIONREGEX_H
#define EXPRESSIONREGEX_H

#include <QLatin1String>

class ExpressionRegex
{
public:
    static constexpr QLatin1String cOperatorcharacters{ R"([\+\-\*\^\/\?\<\>\=\!\%\&\|\~\'\_])" };
    static constexpr QLatin1String cNumberDec{ R"(\d+)" };
    static constexpr QLatin1String cNumberHex{ R"(0[x]\d+)" };
    static constexpr QLatin1String cNumberBin{ R"(0[b]\d+)" };

    static constexpr QLatin1String cMatchRegister{ R"(\$\{([^}]+)\})" };
    static constexpr QLatin1String cParseReg{ R"(\$\{\s*([^@:}\s][^@:}]*?)(?:\s*@\s*(\d+))?(?:\s*\:\s*(\w+))?\s*\})" };

private:
};

#endif // EXPRESSIONREGEX_H
