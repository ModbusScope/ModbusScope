#include "readregisters.h"

using State = ResultState::State;

ReadRegisters::ReadRegisters()
{

}

/*!
 * Load ReadRegisterCollection with register read list
 * \param registerList  Register read list
 * \param consecutiveMax Number of consecutive registers that is allowed to read at once
 */
void ReadRegisters::resetRead(QList<ModbusDataUnit> registerList, quint16 consecutiveMax)
{
    _resultMap.clear();

    if (registerList.size() == 0)
    {
        _readItemList.clear();
    }

    while(registerList.size() > 0)
    {
        if (
            (registerList.size() == 1)
            || (consecutiveMax == 1)
        )
        {
            // Only single reads allowed
            _readItemList.append(ModbusReadItem(registerList.first(), 1));

            registerList.removeFirst();
        }
        else
        {
            int currentIdx = 0;

            bool bSubsequent = false;
            do
            {

                // if next is current + 1, dan subsequent = true
                if (registerList.at(currentIdx).next() == registerList.at(currentIdx + 1))
                {
                    bSubsequent = true;
                    currentIdx++;
                }
                else
                {
                    bSubsequent = false;
                }

                // Break loop when end of list
                if (currentIdx >= (registerList.size() - 1))
                {
                    break;
                }

                // Limit number of register in 1 read
                if ((currentIdx + 1) >= consecutiveMax)
                {
                    break;
                }

            } while(bSubsequent == true);


            // Convert idx to count
            quint8 count = static_cast<quint8>(currentIdx) + 1;

            _readItemList.append(ModbusReadItem(registerList.first(), count));

            for (int idx = 0; idx < count; idx++)
            {
                registerList.removeFirst();
            }
        }
    }
}

/*!
 * Return whether there is still a ModbusReadItem left
 * \retval true     Still ModbusReadItemLeft
 * \retval false    None left
 */
bool ReadRegisters::hasNext()
{
    return !_readItemList.isEmpty();
}

/*!
 * Get next ModbusReadItem
 * \return next ModbusReadItem (item with count 0 when no item available)
 */
ModbusReadItem ReadRegisters::next()
{
    if (hasNext())
    {
        return _readItemList.first();
    }
    else
    {
        return ModbusReadItem(ModbusDataUnit(0), 0);
    }
}

/*!
 * Add success result for current ReadRegister cluster
 * \param startRegister     Start register address
 * \param registerDataList  List with result data
 */
void ReadRegisters::addSuccess(ModbusDataUnit const& startRegister, QList<quint16> registerDataList)
{
    if (
        hasNext()
        && (next().address() == startRegister)
        && (registerDataList.size() >= next().count())
    )
    {
        for (qint32 i = 0; i < next().count(); i++)
        {
            const auto registerAddr = startRegister.next(i);
            const auto result = Result<quint16>(registerDataList[i], State::SUCCESS);

            _resultMap.insert(registerAddr, result);
        }

        _readItemList.removeFirst();
    }
}

/*!
 * Add error result for current ReadRegister cluster
 */
void ReadRegisters::addError()
{
    if (hasNext())
    {
        auto nextRequestData = next();
        for (quint32 i = 0; i < nextRequestData.count(); i++)
        {
            const auto registerAddr = nextRequestData.address().next(i);
            const auto result = Result<quint16>(0, State::INVALID);

            _resultMap.insert(registerAddr, result);
        }

        _readItemList.removeFirst();
    }
}

/*!
 * Mark all remaining register as errors
 */
void ReadRegisters::addAllErrors()
{
    while(hasNext())
    {
        addError();
    }
}

/*!
 * Split "next" ModbusReadItem into single reads.
 */
void ReadRegisters::splitNextToSingleReads()
{
    if (hasNext())
    {
        ModbusReadItem firstItem = _readItemList.first();

        _readItemList.removeFirst();

        for(int idx = firstItem.count(); idx > 0; idx--)
        {
            _readItemList.prepend(ModbusReadItem(firstItem.address().next(idx - 1), 1));
        }
    }
}

/*!
 * Return result map
 * \return Result map
 */
ModbusResultMap ReadRegisters::resultMap()
{
    return _resultMap;
}
