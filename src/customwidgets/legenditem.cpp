

#include "legenditem.h"
#include <QStyleOption>
#include <QPainter>

LegendItem::LegendItem(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QHBoxLayout(this);

    _pLabelText = new QLabel("", this);

    _pLabelColor = new QLabel(this);
    _pLabelColor->setAutoFillBackground(true);
    _pLabelColor->setMinimumSize(10, 10);
    _pLabelColor->setMaximumSize(10, 10);

    _pLayout->setSpacing(4);
    _pLayout->setContentsMargins(1, 1, 1, 1); // This is redundant with setMargin, which is deprecated

    _pLayout->addWidget(_pLabelColor);
    _pLayout->setStretch(0, 0);

    _pLayout->addWidget(_pLabelText);
    _pLayout->setStretch(1, 1);

    setLayout(_pLayout);
}

void LegendItem::setText(QString txt)
{
    _pLabelText->setText(txt);
}

void LegendItem::setColor(QColor color)
{
    QPalette palette = _pLabelColor->palette();
    palette.setColor(QPalette::Background, color);
    _pLabelColor->setPalette(palette);
}
