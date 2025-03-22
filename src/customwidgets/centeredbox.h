#ifndef CENTEREDBOX_H
#define CENTEREDBOX_H

#include <QProxyStyle>
#include <QStyleOptionViewItem>

// https://wiki.qt.io/Center_a_QCheckBox_or_Decoration_in_an_Itemview

enum
{
    CheckAlignmentRole = Qt::UserRole + Qt::CheckStateRole + Qt::TextAlignmentRole,
    DecorationAlignmentRole = Qt::UserRole + Qt::DecorationRole + Qt::TextAlignmentRole
};

class CenteredBoxProxyStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;
    QRect subElementRect(QStyle::SubElement element, const QStyleOption* option, const QWidget* widget) const override
    {
        const QRect baseRes = QProxyStyle::subElementRect(element, option, widget);
        switch (element)
        {
        case SE_ItemViewItemCheckIndicator:
        {
            const QStyleOptionViewItem* const itemOpt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            const QVariant alignData = itemOpt ? itemOpt->index.data(CheckAlignmentRole) : QVariant();
            if (!alignData.isNull())
                return QStyle::alignedRect(itemOpt->direction, alignData.value<Qt::Alignment>(), baseRes.size(),
                                           itemOpt->rect);
            break;
        }
        case SE_ItemViewItemDecoration:
        {
            const QStyleOptionViewItem* const itemOpt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            const QVariant alignData = itemOpt ? itemOpt->index.data(DecorationAlignmentRole) : QVariant();
            if (!alignData.isNull())
                return QStyle::alignedRect(itemOpt->direction, alignData.value<Qt::Alignment>(), baseRes.size(),
                                           itemOpt->rect);
            break;
        }
        case SE_ItemViewItemFocusRect:
        {
            const QStyleOptionViewItem* const itemOpt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            const QVariant checkAlignData = itemOpt ? itemOpt->index.data(CheckAlignmentRole) : QVariant();
            const QVariant decorationAlignData = itemOpt ? itemOpt->index.data(DecorationAlignmentRole) : QVariant();
            if (!checkAlignData.isNull() ||
                !decorationAlignData
                   .isNull()) // when it is not null, then the focus rect should be drawn over the complete cell
                return option->rect;
            break;
        }
        default:
            break;
        }
        return baseRes;
    }
};

#endif // CENTEREDBOX_H
