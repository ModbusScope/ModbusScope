#ifndef MBCHEADER_H
#define MBCHEADER_H

#include <QHeaderView>
#include <qevent.h>
#include <qpainter.h>

class MbcHeader : public QHeaderView
{
    Q_OBJECT

public:
    MbcHeader(Qt::Orientation orientation, QWidget* parent = nullptr) : QHeaderView(orientation, parent)
    {
    }

signals:
    void selectAllClicked(Qt::CheckState);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override
    {
        painter->save();
        QHeaderView::paintSection(painter, rect, logicalIndex);
        painter->restore();

        if (model() && logicalIndex == 0)
        {
            QStyleOptionButton option;
            option.initFrom(this);

            QRect checkbox_rect = style()->subElementRect(QStyle::SubElement::SE_CheckBoxIndicator, &option, this);
            checkbox_rect.moveCenter(rect.center());

            bool checked = model()->headerData(logicalIndex, orientation(), Qt::CheckStateRole).toBool();

            option.rect = checkbox_rect;
            option.state = checked ? QStyle::State_On : QStyle::State_Off;

            style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        QHeaderView::mouseReleaseEvent(event);
        if (model())
        {
            int section = logicalIndexAt(event->pos());
            if (section == 0)
            {
                const uint state = model()->headerData(section, orientation(), Qt::CheckStateRole).toUInt();
                Qt::CheckState selectAllState = static_cast<Qt::CheckState>(state);
                if ((selectAllState == Qt::Checked) || (selectAllState == Qt::PartiallyChecked))
                {
                    selectAllState = Qt::Unchecked;
                }
                else
                {
                    selectAllState = Qt::Checked;
                }

                emit selectAllClicked(selectAllState);
            }
        }
    }
};

#endif // MBCHEADER_H
