// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "ColorPickerWidget.hpp"

#include <QColorDialog>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>

ColorPickerWidget::ColorPickerWidget(QWidget* parent)
    : QWidget(parent)
{
    update_text();

    auto effect = std::make_unique<QGraphicsDropShadowEffect>();
    effect->setBlurRadius(12.0);
    effect->setColor(QColor{0, 0, 0, 50});
    effect->setOffset(0.0, 0.0);

    setGraphicsEffect(effect.release());
}

QColor ColorPickerWidget::selected_color() const noexcept
{
    return m_current_color;
}

void ColorPickerWidget::set_selected_color(QColor value)
{
    if (selected_color() != value)
    {
        m_current_color = value;
        update_text();
        update();
    }
}

QSize ColorPickerWidget::sizeHint() const
{
    return QFontMetrics{font()}.boundingRect(m_current_text).size();
}

void ColorPickerWidget::set_has_opacity(bool value)
{
    if (m_has_opacity != value)
    {
        m_has_opacity = value;
        update_text();
        update();
    }
}

void ColorPickerWidget::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    QPainter painter{this};
    painter.fillRect(rect(), Qt::transparent);

    painter.setRenderHint(QPainter::Antialiasing);

    const QColor selectedCol = selected_color();

    QPainterPath path{};
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), 3, 3);

    // Opacity texture first.
    if (selectedCol.alpha() < 255)
    {
        QBrush brush{};
        brush.setTexture(QPixmap(":/ui/alphaback.png"));
        painter.fillPath(path, brush);
    }

    QLinearGradient gradient{};
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setStart(0.0F, 0.0F);
    gradient.setFinalStop(0.0F, 1.0F);

    if (isEnabled())
    {
        gradient.setColorAt(0.0F, selectedCol.lighter(130));
        gradient.setColorAt(0.1F, selectedCol);
        gradient.setColorAt(1.0F, selectedCol);

        painter.fillPath(path, gradient);

        painter.setPen(selectedCol.lightnessF() < 0.5 ? Qt::white : Qt::black);
        painter.drawText(rect(), Qt::AlignCenter, m_current_text);
    }
    else
    {
        gradient.setColorAt(0.0F, QColor(120, 121, 125));
        gradient.setColorAt(1.0F, QColor(100, 101, 105));
        painter.fillPath(path, gradient);

        painter.setPen(QColor(150, 151, 155));
        painter.drawText(rect(), Qt::AlignCenter, m_current_text);
    }
}

void ColorPickerWidget::mousePressEvent(QMouseEvent* e)
{
    Q_UNUSED(e);

    const auto previousColor = m_current_color;

    auto dialog = new QColorDialog(this);
    dialog->setModal(true);
    dialog->setCurrentColor(previousColor);

    if (m_has_opacity)
    {
        dialog->setOption(QColorDialog::ShowAlphaChannel);
    }

    connect(dialog,
            &QColorDialog::currentColorChanged,
            this,
            &ColorPickerWidget::on_color_changed_in_dialog);
    if (dialog->exec() == QColorDialog::Rejected)
    {
        on_color_changed_in_dialog(previousColor);
    }
}

void ColorPickerWidget::update_text()
{
    const auto value = selected_color();
    m_current_text =
        QStringLiteral("(%1, %2, %3)").arg(value.red()).arg(value.green()).arg(value.blue());
}

void ColorPickerWidget::on_color_changed_in_dialog(const QColor& color)
{
    m_current_color = color;
    update_text();
    update();
    emit color_changed(color);
}
