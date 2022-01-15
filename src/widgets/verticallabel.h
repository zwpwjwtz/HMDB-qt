#ifndef VERTICALLABEL_H
#define VERTICALLABEL_H

#include <QLabel>

class VerticalLabel : public QLabel
{
    Q_OBJECT

public:
    explicit VerticalLabel(QWidget* parent = nullptr);
    explicit VerticalLabel(const QString& text, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
};

#endif // VERTICALLABEL_H
