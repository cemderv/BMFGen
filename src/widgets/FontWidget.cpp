// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontWidget.hpp"

#include "FontModel.hpp"
#include "FontGenWorkerThread.hpp"
#include "ui_FontWidget.h"
#include "windows/CharacterSetsDialog.hpp"
#include "windows/FontVariationDialog.hpp"
#include <QScrollBar>
#include <QTimer>

FontWidget::FontWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::FontWidget())
{
    ui->setupUi(this);

    ui->lbl_include_kerning->hide();
    ui->chk_include_kerning->hide();
}

FontWidget::~FontWidget() = default;

static Qt::PenJoinStyle index_to_pen_join_style(int index)
{
    switch (index)
    {
        case 0: return Qt::MiterJoin;
        case 1: return Qt::BevelJoin;
        case 2: return Qt::RoundJoin;
        default: return Qt::MiterJoin;
    }
}

static int pen_join_style_to_index(Qt::PenJoinStyle pen_join_style)
{
    switch (pen_join_style)
    {
        case Qt::MiterJoin: return 0;
        case Qt::BevelJoin: return 1;
        case Qt::RoundJoin: return 2;
        default: return 0;
    }
}

void FontWidget::set_font_model(FontModel* font)
{
    Q_ASSERT(m_font == nullptr);

    m_font = font;

    ui->font_selection_widget->set_font_model(m_font);
    ui->text_preview_widget->set_font_model(m_font);

    ui->cmb_max_page_size->setCurrentText(QString::number(m_font->max_page_extent()));
    ui->chk_include_kerning->setChecked(m_font->use_kerning());
    ui->chk_anti_aliasing->setChecked(m_font->anti_aliasing());

    ui->cmb_font_desc_type->setCurrentIndex(static_cast<int>(m_font->desc_type()));
    ui->cmb_image_type->set_font_image_type(m_font->image_type());
    ui->chk_flip_images_upside_down->setChecked(m_font->should_flip_images_upside_down());
    ui->chk_allow_monochromatic_images->setChecked(m_font->allow_monochromatic_images());
    ui->txt_output_directory->setText(m_font->export_directory());

    ui->fill_options_widget->set_font_model(m_font, m_font->base_fill());
    ui->stroke_fill_options_widget->set_font_model(m_font, m_font->stroke_fill());

    ui->num_stroke_spread->setValue(m_font->stroke_spread());

    ui->cmb_stroke_style->setCurrentIndex(static_cast<int>(m_font->stroke_style()) - 1);
    ui->cmb_stroke_cap_style->setCurrentIndex(static_cast<int>(m_font->stroke_cap_style()));
    ui->cmb_stroke_join_style->setCurrentIndex(
        pen_join_style_to_index(m_font->stroke_join_style()));

    ui->sld_stroke_miter_limit->setValue(m_font->stroke_miter_limit());
    ui->sld_stroke_dash_offset->setValue(m_font->stroke_dash_offset());

    ui->grp_fill->setChecked(m_font->base_fill().fill_type != FillType::None);
    ui->grp_outline->setChecked(m_font->stroke_fill().fill_type != FillType::None);

    ui->fontSettingsScrollArea->setFixedWidth(
        ui->fontSettingsScrollAreaContents->minimumSizeHint().width() +
        ui->fontSettingsScrollArea->horizontalScrollBar()->sizeHint().width() + 50);

    {
        const bool is_visible = m_font->stroke_style() != Qt::PenStyle::SolidLine;
        ui->lbl_stroke_dash_offset->setVisible(is_visible);
        ui->sld_stroke_dash_offset->setVisible(is_visible);
    }
    {
        const bool is_visible = m_font->stroke_join_style() == Qt::MiterJoin;
        ui->lbl_stroke_miter_limit->setVisible(is_visible);
        ui->sld_stroke_miter_limit->setVisible(is_visible);
    }

    update_visibility_of_font_desc_type_dependent_widgets();
}

void FontWidget::on_max_page_extent_changed()
{
    const int value = ui->cmb_max_page_size->currentText().toInt();
    qDebug("Changed max page extent to: %d", value);
    m_font->set_max_page_extent(value);
}

void FontWidget::on_edit_font_variations_clicked()
{
    FontVariationDialog dialog{this, m_font};
    dialog.exec();
}

void FontWidget::on_use_kerning_changed(int state)
{
    qDebug("Changed usage of kerning to: %d", state);
    m_font->set_use_kerning(state == Qt::CheckState::Checked);
}

void FontWidget::on_stroke_style_changed(int index)
{
    Q_UNUSED(index);
    qDebug("Changed stroke style to: %s", qPrintable(ui->cmb_stroke_style->currentText()));
    m_font->set_stroke_style(static_cast<Qt::PenStyle>(ui->cmb_stroke_style->currentIndex() + 1));

    const bool is_visible = m_font->stroke_style() != Qt::PenStyle::SolidLine;
    ui->lbl_stroke_dash_offset->setVisible(is_visible);
    ui->sld_stroke_dash_offset->setVisible(is_visible);
}

void FontWidget::on_anti_aliasing_changed()
{
    m_font->set_anti_aliasing(ui->chk_anti_aliasing->isChecked());
}

void FontWidget::on_stroke_cap_style_changed(int index)
{
    Q_UNUSED(index);
    qDebug("Changed stroke cap style to: %s", qPrintable(ui->cmb_stroke_cap_style->currentText()));
    m_font->set_stroke_cap_style(
        static_cast<Qt::PenCapStyle>(ui->cmb_stroke_cap_style->currentIndex()));
}

void FontWidget::on_stroke_join_style_changed(int index)
{
    Q_UNUSED(index);

    qDebug("Changed stroke join style to: %s",
           qPrintable(ui->cmb_stroke_join_style->currentText()));

    m_font->set_stroke_join_style(
        index_to_pen_join_style(ui->cmb_stroke_join_style->currentIndex()));

    const bool is_visible = m_font->stroke_join_style() == Qt::MiterJoin;

    ui->lbl_stroke_miter_limit->setVisible(is_visible);
    ui->sld_stroke_miter_limit->setVisible(is_visible);
}

void FontWidget::on_stroke_spread_changed()
{
    const int value = ui->num_stroke_spread->value();
    qDebug("Changed stroke spread to: %d", value);
    m_font->set_stroke_spread(value);
}

void FontWidget::on_stroke_miter_limit_changed(int value)
{
    qDebug("Changed stroke miter limit to: %d", value);
    m_font->set_stroke_miter_limit(value);
}

void FontWidget::on_stroke_dash_offset_changed(int value)
{
    qDebug("Changed stroke dash offset to: %d", value);
    m_font->set_stroke_dash_offset(value);
}

void FontWidget::on_base_fill_changed(const FontFill& fill)
{
    qDebug("Base fill changed");
    m_font->set_base_fill(fill);
}

void FontWidget::on_stroke_fill_changed(const FontFill& fill)
{
    qDebug("Stroke fill changed");
    m_font->set_stroke_fill(fill);
}

void FontWidget::on_font_desc_type_changed()
{
    qDebug("Font desc type changed");
    m_font->set_desc_type(static_cast<FontDescriptionType>(ui->cmb_font_desc_type->currentIndex()));
    update_visibility_of_font_desc_type_dependent_widgets();
}

void FontWidget::on_font_image_type_changed()
{
    qDebug("Font image type changed");
    m_font->set_image_type(ui->cmb_image_type->image_type());
}

void FontWidget::on_flip_images_upside_down_changed()
{
    qDebug("Flip images upside down changed");
    m_font->set_should_flip_images_upside_down(ui->chk_flip_images_upside_down->isChecked());
}

void FontWidget::on_allow_monochromatic_images_changed()
{
    qDebug("Allow monochromatic images changed");
    m_font->set_allow_monochromatic_images(ui->chk_allow_monochromatic_images->isChecked());
}

void FontWidget::on_output_directory_changed()
{
    qDebug("Font output directory changed");
    m_font->set_export_directory(ui->txt_output_directory->text());
}

void FontWidget::on_edit_char_sets_clicked()
{
    CharacterSetsDialog dialog{this, m_font};
    dialog.exec();
}

void FontWidget::on_fill_header_check_changed(bool value)
{
    qDebug("Fill checked changed");
    ui->fill_options_widget->set_fill_enabled(value);
}

void FontWidget::on_outline_header_check_changed(bool value)
{
    qDebug("Outline checked changed");
    ui->stroke_fill_options_widget->set_fill_enabled(value);
}

void FontWidget::update_visibility_of_font_desc_type_dependent_widgets()
{
    constexpr auto visible = true;

    ui->lbl_image_type->setVisible(visible);
    ui->cmb_image_type->setVisible(visible);

    ui->lbl_flip_images_upside_down->setVisible(visible);
    ui->chk_flip_images_upside_down->setVisible(visible);

    ui->lbl_allow_mono->setVisible(visible);
    ui->chk_allow_monochromatic_images->setVisible(visible);

    ui->lbl_output_directory->setVisible(visible);
    ui->txt_output_directory->setVisible(visible);
}
