/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "widgets/ColorWheelWidget.hpp"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>

static constexpr double SelectorRadius = 4;
static constexpr int    MaxSize        = 256;

inline QColor RainbowHsv(float hue)
{
    return QColor::fromHsvF(hue, 1, 1);
}

ColorWheelWidget::ColorWheelWidget(QWidget* parent)
    : QWidget(parent)
    , m_hue(0)
    , m_saturation(0)
    , m_value(0)
    , m_is_background_dark(false)
    , m_wheel_width(20)
    , m_mouse_status(MouseStatus::Nothing)
{
    qreal backgroundValue = palette().window().color().valueF();
    m_is_background_dark  = backgroundValue < 0.5;
}

ColorWheelWidget::~ColorWheelWidget() = default;

QColor ColorWheelWidget::color() const
{
    return QColor::fromHsvF(m_hue, m_saturation, m_value, 1.0F);
}

QSize ColorWheelWidget::sizeHint() const
{
    return QSize{int(m_wheel_width * 5), int(m_wheel_width * 5)};
}

qreal ColorWheelWidget::hue() const
{
    return m_hue;
}

qreal ColorWheelWidget::saturation() const
{
    return color().hsvSaturationF();
}

qreal ColorWheelWidget::value() const
{
    return color().valueF();
}

unsigned int ColorWheelWidget::wheel_width() const
{
    return m_wheel_width;
}

void ColorWheelWidget::set_wheel_width(unsigned int w)
{
    m_wheel_width = w;
    render_inner_selector();
    update();
    Q_EMIT wheel_width_changed(w);
}

void ColorWheelWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.translate(geometry().width() / 2, geometry().height() / 2);

    // hue wheel
    if (m_hue_ring.isNull())
    {
        render_ring();
    }

    painter.drawPixmap(-outer_radius(), -outer_radius(), m_hue_ring);

    // hue selector
    draw_ring_editor(m_hue, painter, Qt::black);

    // lum-sat square
    if (m_inner_selector.isNull())
    {
        render_inner_selector();
    }

    painter.rotate(selector_image_angle());
    painter.translate(selector_image_offset());

    QPointF selectorPosition;
    qreal   side   = triangle_side();
    qreal   height = triangle_height();
    qreal   sliceH = side * m_value;
    qreal   ymin   = side / 2 - sliceH / 2;

    selectorPosition = QPointF(m_value * height, ymin + m_saturation * sliceH);
    QPolygonF triangle;
    triangle.append(QPointF{0, side / 2});
    triangle.append(QPointF{height, 0});
    triangle.append(QPointF{height, side});

    QPainterPath clip;
    clip.addPolygon(triangle);
    painter.setClipPath(clip);

    painter.drawImage(QRectF{QPointF{}, selector_size()}, m_inner_selector);
    painter.setClipping(false);

    // lum-sat selector
    // we define the color of the selecto based on the background color of the
    // widget in order to improve to contrast
    if (m_is_background_dark)
    {
        bool isWhite = (m_value < 0.65 || m_saturation > 0.43);
        painter.setPen(QPen(isWhite ? Qt::white : Qt::black, 2));
    }
    else
    {
        painter.setPen(QPen(m_value > 0.5 ? Qt::black : Qt::white, 2));
    }
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(selectorPosition, SelectorRadius, SelectorRadius);
}

void ColorWheelWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (m_mouse_status == MouseStatus::DragCircle)
    {
        auto hue = line_to_point(ev->pos()).angle() / 360.0;
        m_hue    = hue;
        render_inner_selector();

        Q_EMIT this->ColorSelected(color());
        Q_EMIT this->color_changed(color());
        this->update();
    }
    else if (m_mouse_status == MouseStatus::DragSquare)
    {
        QLineF globMouseLn = this->line_to_point(ev->pos());
        QLineF centerMouseLn(QPointF(0, 0), globMouseLn.p2() - globMouseLn.p1());

        centerMouseLn.setAngle(centerMouseLn.angle() + this->selector_image_angle());
        centerMouseLn.setP2(centerMouseLn.p2() - this->selector_image_offset());

        QPointF pt = centerMouseLn.p2();

        qreal side   = this->triangle_side();
        m_value      = qBound(0.0, pt.x() / this->triangle_height(), 1.0);
        qreal sliceH = side * m_value;

        qreal ycenter = side / 2;
        qreal ymin    = ycenter - sliceH / 2;

        if (sliceH > 0)
        {
            m_saturation = qBound(0.0, (pt.y() - ymin) / sliceH, 1.0);
        }

        Q_EMIT ColorSelected(color());
        Q_EMIT color_changed(color());
        update();
    }
}

void ColorWheelWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->buttons() & Qt::LeftButton)
    {
        setFocus();
        QLineF ray = this->line_to_point(ev->pos());
        if (ray.length() <= this->inner_radius())
        {
            m_mouse_status = MouseStatus::DragSquare;
        }
        else if (ray.length() <= this->outer_radius())
        {
            m_mouse_status = MouseStatus::DragCircle;
        }

        // Update the color
        mouseMoveEvent(ev);
    }
}

void ColorWheelWidget::mouseReleaseEvent(QMouseEvent* event)
{
    mouseMoveEvent(event);
    m_mouse_status = MouseStatus::Nothing;
}

void ColorWheelWidget::resizeEvent(QResizeEvent* /*event*/)
{
    this->render_ring();
    this->render_inner_selector();
}

void ColorWheelWidget::set_color(QColor c)
{
    qreal oldh = m_hue;
    this->set_selected_color(c);
    if (!qFuzzyCompare(oldh + 1, m_hue + 1))
    {
        this->render_inner_selector();
    }
    update();
    Q_EMIT color_changed(c);
}

void ColorWheelWidget::set_hue(qreal h)
{
    m_hue = qBound(0.0, h, 1.0);
    this->render_inner_selector();
    update();
}

void ColorWheelWidget::set_saturation(qreal s)
{
    m_saturation = qBound(0.0, s, 1.0);
    update();
}

void ColorWheelWidget::set_value(qreal v)
{
    m_value = qBound(0.0, v, 1.0);
    update();
}

qreal ColorWheelWidget::outer_radius() const
{
    return qMin(this->geometry().width(), this->geometry().height()) / 2;
}

qreal ColorWheelWidget::inner_radius() const
{
    return outer_radius() - m_wheel_width;
}

qreal ColorWheelWidget::square_size() const
{
    return inner_radius() * qSqrt(2);
}

qreal ColorWheelWidget::triangle_height() const
{
    return inner_radius() * 3 / 2;
}

qreal ColorWheelWidget::triangle_side() const
{
    return inner_radius() * qSqrt(3);
}

QLineF ColorWheelWidget::line_to_point(const QPoint& p) const
{
    return {static_cast<qreal>(this->geometry().width() / 2),
            static_cast<qreal>(this->geometry().height() / 2),
            static_cast<qreal>(p.x()),
            static_cast<qreal>(p.y())};
}

void ColorWheelWidget::init_buffer(QSize size)
{
    std::size_t linearSize = size.width() * size.height();
    if (m_inner_selector_buffer.size() == linearSize)
    {
        return;
    }
    m_inner_selector_buffer.resize(linearSize);
    m_inner_selector = QImage(reinterpret_cast<uchar*>(m_inner_selector_buffer.data()),
                              size.width(),
                              size.height(),
                              QImage::Format_RGB32);
}

void ColorWheelWidget::render_inner_selector()
{
    QSizeF size = selector_size();
    if (size.height() > MaxSize)
    {
        size *= MaxSize / size.height();
    }

    const qreal ycenter = size.height() / 2;

    const QSize isize = size.toSize();
    init_buffer(isize);

    for (int x = 0; x < isize.width(); x++)
    {
        qreal pval   = x / size.height();
        qreal sliceH = size.height() * pval;
        for (int y = 0; y < isize.height(); y++)
        {
            qreal ymin  = ycenter - sliceH / 2;
            qreal psat  = qBound(0.0, (y - ymin) / sliceH, 1.0);
            QRgb  color = QColor::fromHsvF(m_hue, psat, pval, 1.0F).rgb();
            m_inner_selector_buffer[isize.width() * y + x] = color;
        }
    }
}

QPointF ColorWheelWidget::selector_image_offset() const
{
    return {-inner_radius(), -triangle_side() / 2};
}

QSizeF ColorWheelWidget::selector_size() const
{
    return {triangle_height(), triangle_side()};
}

qreal ColorWheelWidget::selector_image_angle() const
{
    return -m_hue * 360 - 60;
}

void ColorWheelWidget::render_ring()
{
    const qreal outer_radius = this->outer_radius();

    m_hue_ring = QPixmap(outer_radius * 2, outer_radius * 2);
    m_hue_ring.fill(Qt::transparent);

    QPainter painter{&m_hue_ring};
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    QConicalGradient gradient_hue(0, 0, 0);

    if (constexpr int hue_stops = 24; gradient_hue.stops().size() < hue_stops)
    {
        for (double a = 0; a < 1.0; a += 1.0 / (hue_stops - 1))
        {
            gradient_hue.setColorAt(a, RainbowHsv(a));
        }
        gradient_hue.setColorAt(1, RainbowHsv(0));
    }

    painter.translate(outer_radius, outer_radius);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(gradient_hue));
    painter.drawEllipse(QPointF(0, 0), outer_radius, outer_radius);

    const qreal inner_radius = this->inner_radius();

    painter.setBrush(Qt::transparent);
    painter.drawEllipse(QPointF(0, 0), inner_radius, inner_radius);
}

void ColorWheelWidget::set_selected_color(const QColor& c)
{
    m_hue        = qMax(0.0, c.hsvHueF());
    m_saturation = c.hsvSaturationF();
    m_value      = c.valueF();
}

void ColorWheelWidget::draw_ring_editor(double editorHue, QPainter& painter, QColor color) const
{
    painter.setPen(QPen(color, 2));
    painter.setBrush(Qt::NoBrush);

    QLineF ray(0, 0, outer_radius(), 0);
    ray.setAngle(editorHue * 360);

    QPointF h1 = ray.p2();
    ray.setLength(inner_radius());

    QPointF h2 = ray.p2();
    painter.drawLine(h1, h2);
}

void ColorWheelWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasColor() ||
        (event->mimeData()->hasText() && QColor(event->mimeData()->text()).isValid()))
    {
        event->acceptProposedAction();
    }
}

void ColorWheelWidget::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasColor())
    {
        this->set_selected_color(event->mimeData()->colorData().value<QColor>());
        event->accept();
    }
    else if (event->mimeData()->hasText())
    {
        QColor col(event->mimeData()->text());
        if (col.isValid())
        {
            this->set_selected_color(col);
            event->accept();
        }
    }
}
