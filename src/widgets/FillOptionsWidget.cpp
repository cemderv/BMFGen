// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FillOptionsWidget.hpp"

#include "FontModel.hpp"
#include "ui_FillOptionsWidget.h"
#include <QFileDialog>
#include <QGradientStop>
#include <QMessageBox>

FillOptionsWidget::FillOptionsWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::FillOptionsWidget())
    , m_font(nullptr)
{
    ui->setupUi(this);
}

FillOptionsWidget::~FillOptionsWidget() noexcept
{
    delete ui;
}

void FillOptionsWidget::set_font_model(FontModel* font, const FontFill& fill)
{
    Q_ASSERT(m_font == nullptr);

    m_font = font;
    m_fill = fill;

    if (m_fill.fill_type == FillType::None)
    {
        ui->cmb_fill_type->setCurrentIndex(0);
    }
    else
    {
        ui->cmb_fill_type->setCurrentIndex(static_cast<int>(m_fill.fill_type) - 1);
    }

    ui->solid_color_picker->set_selected_color(m_fill.solid_color);
    ui->fill_gradient_picker->set_stops(m_fill.gradient_stops);
    ui->dial_fill_gradient_angle->setValue(m_fill.gradient_angle);
    ui->sld_fill_radial_gradient_offset_x->setValue(m_fill.gradient_offset.x());
    ui->sld_fill_radial_gradient_offset_y->setValue(m_fill.gradient_offset.y());
    ui->sld_fill_radial_gradient_radius->setValue(m_fill.gradient_radius);

    QPixmap pixmap;
    {
        auto img = QImage(font->absolute_filename(fill.image_filename));
        if (!img.isNull())
        {
            pixmap = QPixmap::fromImage(img);
        }
        else
        {
            pixmap = QIcon::fromTheme("image").pixmap(ui->lbl_fill_image_selection->size());
        }
    }

    ui->lbl_fill_image_selection->setPixmap(pixmap);

    update_fill_group_visibility();
}

void FillOptionsWidget::set_fill_enabled(bool value)
{
    const FillType prev_fill_type = m_fill.fill_type;

    if (value)
    {
        m_fill.fill_type = static_cast<FillType>(ui->cmb_fill_type->currentIndex() + 1);
    }
    else
    {
        m_fill.fill_type = FillType::None;
    }

    if (m_fill.fill_type != prev_fill_type)
    {
        emit fill_changed(m_fill);
    }
}

void FillOptionsWidget::on_fill_type_changed()
{
    qDebug("Changed fill type to: %s", qPrintable(ui->cmb_fill_type->currentText()));
    m_fill.fill_type = static_cast<FillType>(ui->cmb_fill_type->currentIndex() + 1);
    update_fill_group_visibility();
    emit fill_changed(m_fill);
}

void FillOptionsWidget::on_open_fill_image()
{
    const QString filename =
        QFileDialog::getOpenFileName(this,
                                     QStringLiteral("Open"),
                                     QString(),
                                     QStringLiteral("Image file (*.png *.jpg *.jpeg *.bmp)"),
                                     nullptr);

    if (filename.isEmpty())
    {
        return;
    }

    const QPixmap img{filename};

    if (img.isNull())
    {
        QMessageBox::critical(this,
                              QStringLiteral("Error"),
                              QStringLiteral("Failed to open the image file."),
                              QMessageBox::Ok);
        return;
    }

    ui->lbl_fill_image_selection->setPixmap(img);

    m_fill.image_filename = m_font->relative_filename(filename);

    emit fill_changed(m_fill);
}

void FillOptionsWidget::update_fill_group_visibility()
{
    for (QObject* child : children())
    {
        if (const auto widget = qobject_cast<QWidget*>(child); widget != nullptr)
        {
            widget->hide();
        }
    }

    ui->lbl_fill_type->show();
    ui->cmb_fill_type->show();

    if (m_fill.fill_type == FillType::SolidColor)
    {
        ui->solid_color_picker->show();
    }
    else if (m_fill.fill_type == FillType::LinearGradient)
    {
        ui->fill_gradient_picker->show();
        ui->dial_fill_gradient_angle->show();
        ui->lbl_gradient_angle_value->show();
        ui->lbl_fill_gradient_angle->show();
    }
    else if (m_fill.fill_type == FillType::RadialGradient)
    {
        ui->fill_gradient_picker->show();

        ui->lbl_fill_radial_gradient_offset_x->show();
        ui->sld_fill_radial_gradient_offset_x->show();

        ui->lbl_fill_radial_gradient_offset_y->show();
        ui->sld_fill_radial_gradient_offset_y->show();

        ui->lbl_fill_radial_gradient_radius->show();
        ui->sld_fill_radial_gradient_radius->show();
    }
    else if (m_fill.fill_type == FillType::Image)
    {
        ui->lbl_fill_image->show();
        ui->lbl_fill_image_selection->show();
    }
}

void FillOptionsWidget::on_solid_color_changed(QColor color)
{
    qDebug("Changed solid color to: %d; %d; %d", color.red(), color.green(), color.blue());
    m_fill.solid_color = color;
    emit fill_changed(m_fill);
}

void FillOptionsWidget::on_gradient_changed(const QGradientStops& stops)
{
    qDebug("Gradient stops changed");
    m_fill.gradient_stops = stops;
    emit fill_changed(m_fill);
}

void FillOptionsWidget::on_gradient_angle_changed(int value)
{
    qDebug("Changed gradient angle to: %d", value);
    m_fill.gradient_angle = value;
    ui->lbl_gradient_angle_value->setText(QStringLiteral("%1°").arg(m_fill.gradient_angle));
    emit fill_changed(m_fill);
}

void FillOptionsWidget::on_gradient_offset_x_changed(int value)
{
    qDebug("Changed gradient offset X to: %d", value);
    m_fill.gradient_offset.setX(value);
    emit fill_changed(m_fill);
}

void FillOptionsWidget::on_gradient_offset_y_changed(int value)
{
    qDebug("Changed gradient offset Y to: %d", value);
    m_fill.gradient_offset.setY(value);
    emit fill_changed(m_fill);
}

void FillOptionsWidget::on_gradient_radius_changed(int value)
{
    qDebug("Changed gradient radius to: %d", value);
    m_fill.gradient_radius = value;
    emit fill_changed(m_fill);
}
