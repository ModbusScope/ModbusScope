/*

	 _____  __ _____________ _______  ______ ___________
	/     \|  |  \____ \__  \\_  __ \/  ___// __ \_  __ \
   |  Y Y  \  |  /  |_> > __ \|  | \/\___ \\  ___/|  | \/
   |__|_|  /____/|   __(____  /__|  /____  >\___  >__|
		 \/      |__|       \/           \/     \/
   Copyright (C) 2004 - 2020 Ingo Berg

	Redistribution and use in source and binary forms, with or without modification, are permitted
	provided that the following conditions are met:

	  * Redistributions of source code must retain the above copyright notice, this list of
		conditions and the following disclaimer.
	  * Redistributions in binary form must reproduce the above copyright notice, this list of
		conditions and the following disclaimer in the documentation and/or other materials provided
		with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
	OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MU_PARSER_REGISTER_H
#define MU_PARSER_REGISTER_H

//--- Standard includes ------------------------------------------------------------------------
#include <vector>

//--- Parser includes --------------------------------------------------------------------------
#include "muparser/include/muParserBase.h"

/** \file
    \brief Definition of a modbus register expression parser.
*/

namespace mu
{
	/** \brief Mathematical expressions parser.
	*/
class API_EXPORT_CXX ParserRegister final : public ParserBase
{
public:
    ParserRegister();
    ParserRegister(const ParserRegister& a_Parser);

    typedef void (*registerCb_t)(int, double*, bool*);

    static void setRegisterCallback(registerCb_t registerCb);

    virtual void InitCharSets();
    virtual void InitFun();
    virtual void InitConst();
        virtual void InitOprt();

    private:

        static int64_t ConvertToInteger(value_type v) { return (int64_t)(floor(v)); };

        static value_type Shr(value_type v1, value_type v2);
        static value_type Shl(value_type v1, value_type v2);
    static value_type LogAnd(value_type v1, value_type v2);
    static value_type LogOr(value_type v1, value_type v2);
    static value_type Not(value_type v1);
    static value_type Mod(value_type v1, value_type v2);
    static value_type RegVal(value_type v1);

    static int IsVal(const char_type* a_szExpr, int* a_iPos, value_type* a_fVal);
    static int IsHexVal(const char_type* a_szExpr, int* a_iPos, value_type* a_iVal);
        static int IsBinVal(const char_type* a_szExpr, int* a_iPos, value_type* a_fVal);

        static registerCb_t _registerCb;

	};
} // namespace mu

#endif

