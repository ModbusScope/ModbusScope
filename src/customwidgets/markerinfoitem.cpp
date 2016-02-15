#include "markerinfoitem.h"

MarkerInfoItem::MarkerInfoItem(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QHBoxLayout(this);
    _pLabel = new QLabel("Test", this);

    _pLayout->addWidget(_pLabel);

    setLayout(_pLayout);
}

