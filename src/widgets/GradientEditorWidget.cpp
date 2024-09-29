// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "widgets/GradientEditorWidget.hpp"
#include <QApplication>
#include <QColorDialog>
#include <QDragEnterEvent>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionSlider>
#include <cmath>
#include <gsl/pointers>

static constexpr int   bar_width = 8;
static constexpr qreal margin    = 4;

inline QColor blend_colors(const QColor& a, const QColor& b, qreal ratio)
{
    return QColor::fromRgbF(a.redF() * (1 - ratio) + b.redF() * ratio,
                            a.greenF() * (1 - ratio) + b.greenF() * ratio,
                            a.blueF() * (1 - ratio) + b.blueF() * ratio,
                            a.alphaF() * (1 - ratio) + b.alphaF() * ratio);
}

/**
 * \brief Get an insertion point in the gradient
 * \param gradient  gradient stops to look into (must be properly set up)
 * \param factor    value in [0, 1] to get the color for
 * \return A pair whose first element is the index to insert the new value at, and a GradientStop
 */
inline QPair<int, QGradientStop> gradient_blended_color_insert(const QGradientStops& gradient,
                                                               qreal                 factor)
{
    if (gradient.empty())
    {
        return {0, {0, QColor()}};
    }

    if (gradient.size() == 1 || factor <= 0)
    {
        return {0, gradient.front()};
    }

    int           i = 0;
    QGradientStop s1;
    for (auto s2 : gradient)
    {
        if (factor < s2.first)
        {
            qreal ratio = (factor - s1.first) / (s2.first - s1.first);
            return {i, {factor, blend_colors(s1.second, s2.second, ratio)}};
        }
        s1 = s2;
        ++i;
    }

    return {gradient.size(), gradient.back()};
}

GradientEditorWidget::GradientEditorWidget(QWidget* parent)
    : QWidget(parent)
    , m_back(Qt::darkGray, Qt::DiagCrossPattern)
    , m_highlighted(-1)
    , m_selected(-1)
    , m_drop_index(-1)
    , m_dialog_selected(-1)
    , m_color_dialog(new QColorDialog(this))
{
    m_back.setTexture(QPixmap{QStringLiteral(":/color_widgets/alphaback.png")});
    m_gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    m_gradient.setSpread(QGradient::RepeatSpread);

    setMouseTracking(true);
    resize(QWidget::sizeHint());
    setAcceptDrops(true);

    auto effect = std::make_unique<QGraphicsDropShadowEffect>();
    effect->setBlurRadius(3.0);
    effect->setColor(QColor{0, 0, 0, 120});
    effect->setOffset(0.0, 0.5);

    setGraphicsEffect(effect.release());
}

GradientEditorWidget::~GradientEditorWidget() = default;

void GradientEditorWidget::refresh_gradient()
{
    m_gradient.setStops(m_stops);
}

qreal GradientEditorWidget::paint_pos(const QGradientStop& stop) const
{
    return margin + stop.first * (geometry().width() - 5);
}

int GradientEditorWidget::closest(QPoint p)
{
    if (m_stops.isEmpty())
    {
        return -1;
    }

    if (m_stops.size() == 1 || geometry().width() <= 5)
    {
        return -1;
    }

    const qreal pos = p.x();

    for (int i = 0; i < m_stops.size(); ++i)
    {
        const qreal pos2 = paint_pos(m_stops[i]);

        if (pos >= pos2 - bar_width && pos <= pos2 + bar_width)
        {
            return i;
        }
    }

    return -1;
}

qreal GradientEditorWidget::move_pos(QPoint p) const
{
    const int   width = geometry().width();
    const qreal x     = p.x();

    return (width > 5) ? qMax(qMin((x - margin) / (width - 5), 1.0), 0.0) : 0;
}

void GradientEditorWidget::add_stop_data(int& index, qreal& pos, QColor& color)
{
    if (m_stops.empty())
    {
        index = 0;
        pos   = 0;
        color = Qt::black;
        return;
    }
    if (m_stops.size() == 1)
    {
        color = m_stops[0].second;
        if (m_stops[0].first == 1)
        {
            index = 0;
            pos   = 0.5;
        }
        else
        {
            index = 1;
            pos   = (m_stops[0].first + 1) / 2;
        }
        return;
    }

    int i_before = m_selected;
    if (i_before == -1)
    {
        i_before = int(m_stops.size() - 1);
    }

    if (i_before == m_stops.size() - 1)
    {
        if (m_stops[i_before].first < 1)
        {
            color = m_stops[i_before].second;
            pos   = (m_stops[i_before].first + 1) / 2;
            index = int(m_stops.size());
            return;
        }
        i_before--;
    }

    index = i_before + 1;
    pos   = (m_stops[i_before].first + m_stops[i_before + 1].first) / 2;
    color = blend_colors(m_stops[i_before].second, m_stops[i_before + 1].second, 0.5);
}

void GradientEditorWidget::mouseDoubleClickEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        ev->accept();
        if (m_highlighted != -1)
        {
            qreal       highlighted_pos = paint_pos(m_stops[m_highlighted]);
            const qreal mouse_pos       = ev->pos().x();
            qreal       tolerance       = 4;
            if (std::abs(mouse_pos - highlighted_pos) <= tolerance)
            {
                m_dialog_selected = m_highlighted;

                const auto previous_color = m_stops[m_highlighted].second;

                gsl::owner<QColorDialog*> dialog = new QColorDialog(this);
                dialog->setModal(true);
                dialog->setCurrentColor(previous_color);
                dialog->setOption(QColorDialog::ShowAlphaChannel);

                connect(dialog,
                        &QColorDialog::currentColorChanged,
                        this,
                        &GradientEditorWidget::on_color_changed_in_dialog);

                if (dialog->exec() == QColorDialog::Rejected)
                {
                    on_color_changed_in_dialog(previous_color);
                }

                return;
            }
        }

        qreal pos  = move_pos(ev->pos());
        auto  info = gradient_blended_color_insert(m_stops, pos);
        m_stops.insert(info.first, info.second);
        m_selected = m_highlighted = info.first;
        refresh_gradient();
        update();
    }
    else
    {
        QWidget::mousePressEvent(ev);
    }
}

void GradientEditorWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        ev->accept();
        m_selected = m_highlighted = closest(ev->pos());
        update();
    }
    else if (ev->button() == Qt::RightButton)
    {
        ev->accept();
        remove_selected_stop(ev->pos());
    }
    else
    {
        QWidget::mousePressEvent(ev);
    }
}

void GradientEditorWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (((ev->buttons() & Qt::LeftButton != 0U) != 0U) && m_selected != -1)
    {
        ev->accept();
        qreal pos = move_pos(ev->pos());
        if (m_selected > 0 && pos < m_stops[m_selected - 1].first)
        {
            std::swap(m_stops[m_selected], m_stops[m_selected - 1]);
            m_selected--;
        }
        else if (m_selected < m_stops.size() - 1 && pos > m_stops[m_selected + 1].first)
        {
            std::swap(m_stops[m_selected], m_stops[m_selected + 1]);
            m_selected++;
        }
        m_highlighted             = m_selected;
        m_stops[m_selected].first = pos;
        refresh_gradient();
        emit stops_changed(m_stops);
        update();
    }
    else
    {
        m_highlighted = closest(ev->pos());
        update();
    }
}

void GradientEditorWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton && m_selected != -1)
    {
        ev->accept();
        update();
    }
    else
    {
        QWidget::mousePressEvent(ev);
    }
}

void GradientEditorWidget::leaveEvent(QEvent* /*event*/)
{
    m_highlighted = -1;
    update();
}

QGradientStops GradientEditorWidget::stops() const
{
    return m_stops;
}

void GradientEditorWidget::set_stops(const QGradientStops& colors)
{
    m_highlighted = -1;
    m_selected    = -1;
    m_stops       = colors;
    refresh_gradient();
    update();
}

void GradientEditorWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);

    if (this->isEnabled())
    {
        m_gradient.setFinalStop(1, 0);

        // Background
        {
            const auto   rounded_rect = this->rect().adjusted(1, 1, -1, -1);
            QPainterPath path;
            path.addRoundedRect(rounded_rect, 6, 6);

            // Opacity texture first.
            {
                QBrush brush;
                brush.setTexture(QPixmap{":/ui/alphaback.png"});
                painter.fillPath(path, brush);
            }

            painter.fillPath(path, m_gradient);

            QLinearGradient darker_gradient{rounded_rect.topLeft(), rounded_rect.topRight()};

            QGradientStops darker_stops;
            for (const auto& stop : m_stops)
            {
                darker_stops.emplaceBack(stop.first, stop.second.darker(110));
            }

            darker_gradient.setStops(darker_stops);

            painter.setPen(QPen{darker_gradient, 1});
            painter.drawPath(path);
        }

        int i = 0;
        for (const QGradientStop& stop : m_stops)
        {
            QColor color = stop.second;
            color.setAlpha(255);
            QColor border_color = color.darker();

            if (color.valueF() <= 0.5 && color.alphaF() >= 0.5)
            {
                border_color = Qt::white;
            }

            QRectF rr;
            rr.setX(paint_pos(stop) - bar_width * 0.5);
            rr.setY(margin);
            rr.setWidth(bar_width);
            rr.setHeight(geometry().height() - margin * 2);

            rr.setX(std::round(rr.x()));
            rr.setY(std::round(rr.y()));
            rr.setWidth(std::round(rr.width()));
            rr.setHeight(std::round(rr.height()));

            if (i == m_selected)
            {
                QPainterPath path;
                path.addRoundedRect(rr, 2.0, 2.0);
                painter.fillPath(path, color.lighter(200));

                painter.setPen(QPen(border_color, 1));
                painter.drawRoundedRect(rr, 2.0, 2.0);
            }
            else if (i == m_highlighted)
            {
                QPainterPath path;
                path.addRoundedRect(rr, 2.0, 2.0);
                painter.fillPath(path, color.lighter(180));

                painter.setPen(QPen(border_color, 1));
                painter.drawRoundedRect(rr, 2.0, 2.0);
            }
            else
            {
                QPainterPath path;
                path.addRoundedRect(rr, 2.0, 2.0);
                painter.fillPath(path, color.lighter(120));

                painter.setPen(QPen(border_color, 1));
                painter.drawRoundedRect(rr, 2.0, 2.0);
            }

            i++;
        }

        if (m_drop_index != -1 && m_drop_color.isValid())
        {
            qreal pos = m_drop_pos * (geometry().width() - 5);
            painter.setPen(QPen(m_drop_color, 3));
            QPointF p1 = QPointF{margin, margin} + QPointF(pos, 0);
            QPointF p2 = p1 + QPointF(0, geometry().height() - 5);
            painter.drawLine(p1, p2);
        }
    }
    else
    {
        const auto   rounded_rect = this->rect().adjusted(1, 1, -1, -1);
        QPainterPath path;
        path.addRoundedRect(rounded_rect, 3, 3);

        QLinearGradient gradient;
        gradient.setCoordinateMode(QGradient::ObjectMode);
        gradient.setStart(0.0F, 0.0F);
        gradient.setFinalStop(1.0F, 0.0F);
        gradient.setColorAt(0.0F, QColor{120, 121, 125});
        gradient.setColorAt(1.0F, QColor{100, 101, 105});
        painter.fillPath(path, gradient);
    }
}

void GradientEditorWidget::remove_selected_stop(QPoint point)
{
    if (m_stops.size() <= 2)
    {
        return;
    }

    m_selected = m_highlighted = this->closest(point);

    int selected = m_selected;
    if (selected == -1)
    {
        selected = int(m_stops.size()) - 1;
    }

    m_stops.remove(selected);
    this->refresh_gradient();

    if (m_selected != -1)
    {
        m_selected = -1;
    }

    this->update();
    emit this->stops_changed(m_stops);
}

void GradientEditorWidget::on_color_changed_in_dialog(const QColor& color)
{
    m_stops[m_dialog_selected].second = color;
    this->refresh_gradient();
    Q_EMIT stops_changed(m_stops);
    this->update();
}
