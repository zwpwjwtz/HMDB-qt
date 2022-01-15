#include <QPainter>
#include "verticallabel.h"


VerticalLabel::VerticalLabel(QWidget *parent)
    : QLabel(parent)
{}

VerticalLabel::VerticalLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{}

void VerticalLabel::paintEvent(QPaintEvent*)
{
    Qt::Alignment alignment;
    if (this->alignment() & Qt::AlignVCenter)
        alignment = Qt::AlignHCenter;
    else if (this->alignment() & Qt::AlignHCenter)
        alignment = Qt::AlignVCenter;
    else
        alignment = Qt::AlignTop | Qt::AlignHCenter;

    QPainter painter(this);
    painter.setFont(font());
    painter.translate(0, height());
    painter.rotate(-90);
    painter.drawText(QRect(0, 0, height(), width()),
                     int(alignment), text());
}

QSize VerticalLabel::minimumSizeHint() const
{
    QSize size = QLabel::minimumSizeHint();
    return QSize(size.height(), size.width());
}

QSize VerticalLabel::sizeHint() const
{
    QSize size = QLabel::sizeHint();
    return QSize(size.height(), size.width());
}
