// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "FontGenContext.hpp"
#include "FontModel.hpp"
#include <QSet>
#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class FontWidget;
}
QT_END_NAMESPACE

class QTimer;

enum class FontDescriptionType;
enum class FontExportImageType;
class CharacterSet;
class CharacterSetInfoWidget;
class SharedFontConfig;
class QListWidgetItem;

class FontWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit FontWidget(QWidget* parent);

    ~FontWidget() override;

    void set_font_model(FontModel* font);

  private slots:
    void on_max_page_extent_changed();

    void on_edit_font_variations_clicked();

    void on_use_kerning_changed(int state);

    void on_stroke_style_changed(int index);

    void on_anti_aliasing_changed();

    void on_stroke_cap_style_changed(int index);

    void on_stroke_join_style_changed(int index);

    void on_stroke_spread_changed();

    void on_stroke_miter_limit_changed(int value);

    void on_stroke_dash_offset_changed(int value);

    void on_base_fill_changed(const FontFill& fill);

    void on_stroke_fill_changed(const FontFill& fill);

    void on_font_desc_type_changed();

    void on_font_image_type_changed();

    void on_flip_images_upside_down_changed();

    void on_allow_monochromatic_images_changed();

    void on_output_directory_changed();

    void on_edit_char_sets_clicked();

    void on_fill_header_check_changed(bool value);

    void on_outline_header_check_changed(bool value);

  private:
    void update_visibility_of_font_desc_type_dependent_widgets();

    Ui::FontWidget* ui{};
    FontModel*      m_font{};
};
