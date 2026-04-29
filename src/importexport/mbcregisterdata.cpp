#include "mbcregisterdata.h"

#include <QtMath>

/*!
 * \brief Default constructor — initialises all fields to safe defaults.
 */
MbcRegisterData::MbcRegisterData()
    : _registerAddress(0), _tabIdx(0), _type(ModbusDataType::Type::UNSIGNED_16), _bReadable(true), _decimals(0)
{
}

MbcRegisterData::~MbcRegisterData()
{
}

/*!
 * \brief Convenience constructor for test data and direct construction.
 * \param registerAddress Modbus register address.
 * \param type Data type of the register.
 * \param name Human-readable label.
 * \param tabIdx Index of the tab this register belongs to.
 * \param bReadable Whether the register is readable.
 * \param decimals Number of decimal places for display scaling.
 */
MbcRegisterData::MbcRegisterData(
  quint32 registerAddress, ModbusDataType::Type type, QString name, qint32 tabIdx, bool bReadable, quint8 decimals)
    : _registerAddress(registerAddress),
      _name(name),
      _tabIdx(tabIdx),
      _type(type),
      _bReadable(bReadable),
      _decimals(decimals)
{
}

/*!
 * \brief Compares all fields of this register with another.
 * \param pMbcRegdata Pointer to the register to compare against.
 * \return True when all fields are equal.
 */
bool MbcRegisterData::compare(MbcRegisterData* pMbcRegdata)
{
    bool bRet = true;

    if (_registerAddress != pMbcRegdata->registerAddress())
    {
        bRet = false;
    }

    if (_type != pMbcRegdata->type())
    {
        bRet = false;
    }

    if (_tabIdx != pMbcRegdata->tabIdx())
    {
        bRet = false;
    }

    if (_name != pMbcRegdata->name())
    {
        bRet = false;
    }

    if (_bReadable != pMbcRegdata->isReadable())
    {
        bRet = false;
    }

    if (_decimals != pMbcRegdata->decimals())
    {
        bRet = false;
    }

    return bRet;
}

/*!
 * \brief Returns the Modbus register address.
 */
quint32 MbcRegisterData::registerAddress() const
{
    return _registerAddress;
}

/*!
 * \brief Sets the Modbus register address.
 */
void MbcRegisterData::setRegisterAddress(const quint32& registerAddress)
{
    _registerAddress = registerAddress;
}

/*!
 * \brief Sets the data type of the register.
 */
void MbcRegisterData::setType(ModbusDataType::Type type)
{
    _type = type;
}

/*!
 * \brief Returns the data type of the register.
 */
ModbusDataType::Type MbcRegisterData::type() const
{
    return _type;
}

/*!
 * \brief Returns the human-readable label.
 */
QString MbcRegisterData::name() const
{
    return _name;
}

/*!
 * \brief Sets the human-readable label.
 */
void MbcRegisterData::setName(const QString& name)
{
    _name = name;
}

/*!
 * \brief Returns the tab index this register belongs to.
 */
qint32 MbcRegisterData::tabIdx() const
{
    return _tabIdx;
}

/*!
 * \brief Sets the tab index this register belongs to.
 */
void MbcRegisterData::setTabIdx(const qint32& tabIdx)
{
    _tabIdx = tabIdx;
}

/*!
 * \brief Returns whether the register is readable.
 */
bool MbcRegisterData::isReadable() const
{
    return _bReadable;
}

/*!
 * \brief Sets whether the register is readable.
 */
void MbcRegisterData::setReadable(bool isReadable)
{
    _bReadable = isReadable;
}

/*!
 * \brief Returns the number of decimal places used for display scaling.
 */
quint8 MbcRegisterData::decimals() const
{
    return _decimals;
}

/*!
 * \brief Sets the number of decimal places used for display scaling.
 */
void MbcRegisterData::setDecimals(const quint8& decimals)
{
    _decimals = decimals;
}

/*!
 * \brief Builds the graph expression string for this register.
 *
 * When decimals is non-zero, the expression divides by 10^decimals.
 * \return Expression string suitable for use in GraphData.
 */
QString MbcRegisterData::toExpression() const
{
    const QString typeStr = ModbusDataType::typeString(_type);
    const QString suffix = (typeStr == QStringLiteral("16b")) ? QString() : QString(":%1").arg(typeStr);
    const QString registerStr = QString("${%1%2}").arg(_registerAddress).arg(suffix);

    if (_decimals != 0)
    {
        return QString("%1/%2").arg(registerStr).arg(static_cast<double>(qPow(10, _decimals)));
    }

    return registerStr;
}
