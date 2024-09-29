// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "GeneratedFont.hpp"
#include "QtDataModelUtil.hpp"
#include <QFont>
#include <QGradientStops>
#include <QJsonObject>
#include <QList>
#include <QSet>
#include <optional>

class GlyphsDontFitError : public std::runtime_error
{
  public:
    explicit GlyphsDontFitError()
        : runtime_error("Failed to fit the glyphs into a page.")
    {
    }
};

class InvalidFontFaceError : public std::runtime_error
{
  public:
    explicit InvalidFontFaceError()
        : runtime_error("The font face is invalid.")
    {
    }
};

class NoCharactersSelectedError : public std::runtime_error
{
  public:
    explicit NoCharactersSelectedError()
        : runtime_error("Font generation not possible because no characters are selected.")
    {
    }
};

enum class FillType
{
    None,
    SolidColor,
    LinearGradient,
    RadialGradient,
    Image,
};

struct FontFill
{
    FillType       fill_type{};
    QColor         solid_color;
    QGradientStops gradient_stops;
    int            gradient_angle{};
    QPoint         gradient_offset;
    int            gradient_radius{};
    QString        image_filename;

    bool operator==(const FontFill&) const = default;
    bool operator!=(const FontFill&) const = default;
};

enum class FontDescriptionType;

class FontModel : public QObject
{
    Q_OBJECT

  public:
    explicit FontModel(QObject* parent, const QString& filename);

    ~FontModel() override;

    static std::optional<QString> create_new_font_file_at(const QString& filename);

    QString error_message() const;

    QString name() const;

    QString filename() const;

    std::optional<QString> save_to_file(const QString& filename);

    DEFINE_PROPERTY(QFont, qfont, properties_changed);

    DEFINE_PROPERTY(int, max_page_extent, properties_changed);

    DEFINE_PROPERTY(int, padding, properties_changed);

    DEFINE_PROPERTY(bool, anti_aliasing, properties_changed);

    DEFINE_PROPERTY(bool, use_kerning, properties_changed);

    DEFINE_PROPERTY(FontDescriptionType, desc_type, properties_only_relevant_for_save_changed);

    DEFINE_PROPERTY(FontExportImageType, image_type, properties_only_relevant_for_save_changed);

    DEFINE_PROPERTY(QString, export_directory, properties_only_relevant_for_save_changed);

    DEFINE_PROPERTY(bool,
                    should_flip_images_upside_down,
                    properties_only_relevant_for_save_changed);

    DEFINE_PROPERTY(bool, allow_monochromatic_images, properties_only_relevant_for_save_changed);

    DEFINE_PROPERTY(FontFill, base_fill, properties_changed);

    DEFINE_PROPERTY(FontFill, stroke_fill, properties_changed);

    DEFINE_PROPERTY(int, stroke_spread, properties_changed);

    DEFINE_PROPERTY(Qt::PenStyle, stroke_style, properties_changed);

    DEFINE_PROPERTY(Qt::PenCapStyle, stroke_cap_style, properties_changed);

    DEFINE_PROPERTY(Qt::PenJoinStyle, stroke_join_style, properties_changed);

    DEFINE_PROPERTY(int, stroke_miter_limit, properties_changed);

    DEFINE_PROPERTY(int, stroke_dash_offset, properties_changed);

    DEFINE_PROPERTY(QSet<QChar>, characters, properties_changed);

    DEFINE_PROPERTY(QSet<double>, variations, properties_changed);

    GeneratedFont* generated_font() const;

    void set_generated_font(std::shared_ptr<GeneratedFont> font);

    QColor preview_background_color() const;

    void set_preview_background_color(const QColor& value);

    QString preview_text() const;

    void set_preview_text(const QString& value);

    QString directory() const;

    QString relative_filename(const QString& absolute_filename) const;

    QString absolute_filename(const QString& relative_filename) const;

    explicit operator bool() const;

  signals:
    void properties_changed();

    void properties_only_relevant_for_save_changed();

    void generated_font_changed();

  private:
    std::optional<QString> load_from_file(const QString& filename);

    QString                        m_error_message;
    QString                        m_filename;
    std::shared_ptr<GeneratedFont> m_generated_font;
    QColor                         m_preview_background_color;
    QString                        m_preview_text;
};
