

#include "markerinfo.h"

MarkerInfo::MarkerInfo(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QVBoxLayout();

    _pLayout->setSpacing(0);
    _pLayout->setContentsMargins(0,0,0,0); // This is redundant with setMargin, which is deprecated

    for (quint32 idx = 0; idx < 3; idx++)
    {
        MarkerInfoItem * pItem = new MarkerInfoItem(this);

        _items.append(pItem);
        _pLayout->addWidget(pItem);
    }

    setLayout(_pLayout);
}

