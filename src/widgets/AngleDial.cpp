// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "AngleDial.hpp"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QStyleHints>
#include <QWheelEvent>

static QPointF rotate_around_center(const QPointF& pt,
                                    const QPointF& center,
                                    float          degrees) noexcept
{
    const auto radians = degrees * 0.0174533f;

    const auto c = std::cos(radians);
    const auto s = std::sin(radians);

    const auto ret = pt - center;

    const auto xnew = ret.x() * c - ret.y() * s;
    const auto ynew = ret.x() * s + ret.y() * c;

    return QPointF(xnew + center.x(), ynew + center.y());
}

AngleDial::AngleDial(QWidget* parent)
    : QDial(parent)
{
    auto effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(20.0);
    effect->setColor(QColor(0, 0, 0, 50));
    effect->setOffset(0.0, 0.0);
    setGraphicsEffect(effect);
}

void AngleDial::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    auto painter = QPainter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto myRect = rect();

    auto fillColor    = QColor();
    auto borderColor  = QColor();
    auto pointerColor = QColor();

    if (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark)
    {
        fillColor    = QColor(220, 220, 220);
        borderColor  = QColor(100, 100, 100);
        pointerColor = QColor(70, 70, 70);
    }
    else
    {
        fillColor    = QColor(250, 250, 250);
        borderColor  = QColor(130, 130, 130);
        pointerColor = QColor(100, 100, 100);
    }

    auto pen = QPen();
    pen.setWidth(1);

    auto path = QPainterPath();
    path.addEllipse(myRect.center(), myRect.height() / 2 - 2, myRect.height() / 2 - 2);

    painter.fillPath(path, fillColor);

    pen.setWidth(1);
    pen.setColor(borderColor);

    painter.setPen(pen);
    painter.drawPath(path);

    const auto p1 = QPointF(myRect.center());

    auto p2 = QPointF(p1);
    p2.setY(p2.y() + myRect.height() / 2 - 4);
    p2 = rotate_around_center(p2, p1, float(value()));

    pen.setColor(pointerColor);
    pen.setWidth(2);
    pen.setCapStyle(Qt::RoundCap);

    painter.setPen(pen);
    painter.drawLine(p1, p2);
}

void AngleDial::wheelEvent(QWheelEvent* event)
{
    event->ignore();
}
