// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "FontPage.hpp"
#include "Glyph.hpp"
#include <QSet>

enum class FontDescriptionType
{
    JSON,
    XML,
    Text,
};

enum class FontExportImageType
{
    Png,
    Bmp,
};

extern FontExportImageType font_export_image_type_from_string(const QString& value);

extern QString font_export_image_type_to_string(FontExportImageType type);

class GeneratedFont final
{
  public:
    class Variation
    {
      public:
        const Glyph* find_glyph(QChar code_point) const;

        int pixel_size() const;

        GeneratedFont*   parent_font{};
        double           scale_factor{};
        int              line_height{};
        int              ascent{};
        int              descent{};
        int              line_gap{};
        int              underline_position{};
        QList<qsizetype> glyph_indices;
    };

    GeneratedFont() = default;

    GeneratedFont(QString          name,
                  int              base_size,
                  QSet<QChar>      chars,
                  QList<Glyph>     all_glyphs,
                  QList<Variation> variations,
                  QList<FontPage>  pages);

    bool has_char(QChar character) const;

    int base_size() const;

    const QList<Glyph>& all_glyphs() const;

    const QList<FontPage>& pages() const;

    const FontPage& page_at(int index) const;

    void export_to_disk(const QString&      directory,
                        FontDescriptionType font_description_type,
                        FontExportImageType image_type,
                        bool                flip_images_upside_down,
                        bool                allow_monochromatic_images) const;

    qsizetype size_in_bytes(bool allow_monochromatic_images) const;

    qsizetype total_glyph_count() const;

    int variation_count() const;

    const Variation& default_variation() const;

    const Variation& variation_at(int index) const;

    const Variation& variation_by_scale(double scale) const;

  private:
    std::optional<QStringList> export_images(const QString&      directory,
                                             FontExportImageType type,
                                             bool                flip_upside_down,
                                             bool                allow_monochromatic) const;

    struct ExportArgs
    {
        QString     directory;
        QStringList images_filenames;
    };

    void export_as_json(const ExportArgs& args) const;

    void export_as_xml(const ExportArgs& args) const;

    void export_as_text(const ExportArgs& args) const;

    uint64_t build_cache_key() const;

    QString          m_name;
    int              m_base_size = 0;
    QSet<QChar>      m_chars;
    QList<Glyph>     m_all_glyphs;
    QList<Variation> m_variations;
    QList<FontPage>  m_pages;
};
