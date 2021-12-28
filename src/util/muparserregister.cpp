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

#include "muparserregister.h"

//--- Standard includes ------------------------------------------------------------------------
#include <cmath>
#include <math.h>       /* floor */
#include <algorithm>
#include <numeric>

using namespace std;

/** \file
	\brief Implementation of a modbus register expression parser.
*/



/** \brief Namespace for mathematical applications. */
namespace mu
{

    ParserRegister::registerCb_t ParserRegister::_registerCb;

    value_type ParserRegister::Shr(value_type v1, value_type v2) { return ConvertToInteger(v1) >> ConvertToInteger(v2); }
    value_type ParserRegister::Shl(value_type v1, value_type v2) { return ConvertToInteger(v1) << ConvertToInteger(v2); }
    value_type ParserRegister::LogAnd(value_type v1, value_type v2) { return ConvertToInteger(v1) & ConvertToInteger(v2); }
    value_type ParserRegister::LogOr(value_type v1, value_type v2) { return ConvertToInteger(v1) | ConvertToInteger(v2); }
    value_type ParserRegister::Not(value_type v) { return !ConvertToInteger(v); }
    value_type ParserRegister::Mod(value_type v1, value_type v2) { return ConvertToInteger(v1) % ConvertToInteger(v2); }

    value_type ParserRegister::RegVal(value_type v1)
    {
        double intpart;
        if (modf(v1, &intpart) == 0.0)
        {
            int value = 0;
            bool success = false;
            (*_registerCb)(v1, &value, &success);

            if (success)
            {
                return value;
            }
            else
            {
                throw exception_type(_T("Communication error"));
            }
        }
        else
        {
            throw exception_type(_T("Internal error (invalid index in register list)"));
        }
    }

    //---------------------------------------------------------------------------
    /** \brief Default value recognition callback.
        \param [in] a_szExpr Pointer to the expression
        \param [in, out] a_iPos Pointer to an index storing the current position within the expression
        \param [out] a_fVal Pointer where the value should be stored in case one is found.
        \return 1 if a value was found 0 otherwise.
    */
    int ParserRegister::IsVal(const char_type* a_szExpr, int* a_iPos, value_type* a_fVal)
    {
        value_type fVal(0);

        stringstream_type stream(a_szExpr);
        stream.seekg(0);        // todo:  check if this really is necessary
        stream.imbue(ParserRegister::s_locale);
        stream >> fVal;
        stringstream_type::pos_type iEnd = stream.tellg(); // Position after reading

        if (iEnd == (stringstream_type::pos_type) - 1)
            return 0;

        *a_iPos += (int)iEnd;
        *a_fVal = fVal;
        return 1;
    }

    /** \brief Check a given position in the expression for the presence of
               a hex value.
        \param a_szExpr Pointer to the expression string
        \param [in/out] a_iPos Pointer to an integer value holding the current parsing
               position in the expression.
        \param [out] a_fVal Pointer to the position where the detected value shall be stored.

      Hey values must be prefixed with "0x" in order to be detected properly.
    */
    int ParserRegister::IsHexVal(const char_type* a_szExpr, int* a_iPos, value_type* a_fVal)
    {
        if (a_szExpr[1] == 0 || (a_szExpr[0] != '0' || a_szExpr[1] != 'x'))
            return 0;

        unsigned iVal(0);

        // New code based on streams for UNICODE compliance:
        stringstream_type::pos_type nPos(0);
        stringstream_type ss(a_szExpr + 2);
        ss >> std::hex >> iVal;
        nPos = ss.tellg();

        if (nPos == (stringstream_type::pos_type)0)
            return 1;

        *a_iPos += (int)(2 + nPos);
        *a_fVal = (value_type)iVal;
        return 1;
    }

    int ParserRegister::IsBinVal(const char_type* a_szExpr, int* a_iPos, value_type* a_fVal)
    {
        if (a_szExpr[1] == 0 || (a_szExpr[0] != '0' || a_szExpr[1] != 'b'))
            return 0;

        unsigned int iVal(0),
            iBits(sizeof(iVal) * 8),
            i(0),
            offset(2);

        for (i = 0; (a_szExpr[i + offset] == '0' || a_szExpr[i + offset] == '1') && (i < iBits); ++i)
            iVal |= (int)(a_szExpr[i + offset] == '1') << ((iBits - 1) - i);

        if (i == 0)
            return 0;

        if (i - 1 == iBits)
            throw exception_type(_T("Binary to integer conversion error (overflow)."));

        *a_fVal = (unsigned)(iVal >> (iBits - i));
        *a_iPos += i + offset;

        return 1;
    }

	//---------------------------------------------------------------------------
	/** \brief Constructor.

	  Call ParserBase class constructor and trigger Function, Operator and Constant initialization.
	*/
    ParserRegister::ParserRegister()
		:ParserBase()
	{
        AddValIdent(IsVal);    // lowest priority
        AddValIdent(IsBinVal);
        AddValIdent(IsHexVal); // highest priority

		InitCharSets();
		InitFun();
		InitConst();
		InitOprt();
	}

    ParserRegister::ParserRegister(const ParserRegister& a_Parser)
        :ParserBase(a_Parser)
    {

    }

    void ParserRegister::setRegisterCallback(registerCb_t registerCb)
    {
        _registerCb = registerCb;
    }

	//---------------------------------------------------------------------------
	/** \brief Define the character sets.
		\sa DefineNameChars, DefineOprtChars, DefineInfixOprtChars

	  This function is used for initializing the default character sets that define
	  the characters to be useable in function and variable names and operators.
	*/
	void ParserRegister::InitCharSets()
	{
        DefineNameChars(_T("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));
        DefineOprtChars(_T("+-*^/?<>=!%&|~'_"));
        DefineInfixOprtChars(_T("/+-*^?<>=!%&|~'_"));
    }

	//---------------------------------------------------------------------------
	/** \brief Initialize the default functions. */
	void ParserRegister::InitFun()
	{
        DefineFun(_T("regval"), RegVal);
	}

	//---------------------------------------------------------------------------
	/** \brief Initialize constants.
	*/
	void ParserRegister::InitConst()
	{

	}

	//---------------------------------------------------------------------------
	/** \brief Initialize operators.

	  By default only the unary minus operator is added.
	*/
	void ParserRegister::InitOprt()
	{
        DefineInfixOprt(_T("-"), MathImpl<value_type>::UnaryMinus);
        DefineInfixOprt(_T("+"), MathImpl<value_type>::UnaryPlus);

        DefineInfixOprt(_T("!"), Not);

        DefineOprt(_T("&"), LogAnd, prLOGIC);
        DefineOprt(_T("|"), LogOr, prLOGIC);

        DefineOprt(_T(">>"), Shr, prMUL_DIV + 1);
        DefineOprt(_T("<<"), Shl, prMUL_DIV + 1);

        DefineOprt(_T("%"), Mod, prMUL_DIV);

	}

} // namespace mu
