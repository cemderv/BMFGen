// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "CharacterSet.hpp"
#include "FontPage.hpp"
#include "ImageCache.hpp"
#include "MaxRectsBinPack.hpp"
#include <QHash>
#include <QObject>
#include <optional>

class FontModel;
class GeneratedFont;

class FontGenContext : public QObject
{
    Q_OBJECT

  public:
    explicit FontGenContext(QSet<QChar>* all_chars);

    std::shared_ptr<GeneratedFont> generate_font(const FontModel&                  model,
                                                 const std::optional<QSet<QChar>>& chars_override);

    void cancel();

  private:
    std::shared_ptr<GeneratedFont> generate_bitmap_font(const FontModel&           font,
                                                        std::optional<QSet<QChar>> chars_override);

    std::optional<QList<FontPage>> pack_glyphs(QList<Glyph>& glyphs, int max_page_extent);

    std::optional<QList<FontPage>> create_font_pages(QList<Glyph>& glyphs, int max_page_extent);

    qsizetype insert_as_many_glyphs_as_possible(
        QList<Glyph>&                                        all_glyphs,
        QList<qsizetype>&                                    glyphs_to_insert,
        QSize                                                bin_size,
        binpacking::MaxRectsBinPack::FreeRectChoiceHeuristic heuristic,
        QList<qsizetype>&                                    destination) const;

    ImageCache   m_image_cache;
    bool         m_is_canceled{};
    QSet<QChar>* m_all_characters{};
};
