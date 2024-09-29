// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "StatusLabel.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QTimer>

StatusLabel::StatusLabel(QWidget* parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
    , m_currentDurationMs(0)
{
    setMaximumHeight(25);
    setMinimumWidth(500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    emit visibility_requested(false);

    connect(m_timer, &QTimer::timeout, this, [this] { emit visibility_requested(false); });
}

void StatusLabel::ShowStatus(const QString& text, int durationMs)
{
    m_currentText       = text;
    m_currentDurationMs = durationMs;

    m_timer->start(durationMs);

    emit visibility_requested(true);
    update();
}

void StatusLabel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    const QRect     my_rect          = rect();
    const QPalette& pal              = palette();
    const QColor    background_color = pal.highlight().color();
    const QColor    text_color       = pal.highlightedText().color();
    const float     remaining_time   = remaining_time_in_percent();

    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(remaining_time * 2);

    {
        QPainterPath path;
        path.addRoundedRect(my_rect, 4.0, 4.0);
        painter.fillPath(path, background_color);
    }

    // Animate "time to live" progress bar.
    {
        const QRect activeRect{
            my_rect.x(),
            my_rect.y(),
            static_cast<int>(std::lerp(0.0F, static_cast<float>(my_rect.width()), remaining_time)),
            my_rect.height(),
        };

        QPainterPath path;
        path.addRoundedRect(activeRect, 4.0, 4.0);

        painter.fillPath(path, background_color.lighter());
    }

    painter.setPen(text_color);
    painter.drawText(my_rect, m_currentText, QTextOption{Qt::AlignCenter});

    if (m_timer->isActive())
    {
        update();
    }
}

float StatusLabel::remaining_time_in_percent() const
{
    const int remainingMs = m_timer->remainingTime();

    return static_cast<float>(static_cast<double>(remainingMs) / m_currentDurationMs);
}
