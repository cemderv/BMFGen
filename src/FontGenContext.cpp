// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontGenContext.hpp"

#include "FontModel.hpp"
#include "GeneratedFont.hpp"
#include "ImageCache.hpp"
#include "MaxRectsBinPack.hpp"
#include <QFile>
#include <QPainterPath>
#include <QStaticText>
#include <QTextItem>
#include <array>
#include <optional>

static std::pair<bool, float> can_fit_all_glyphs(
    const QList<Glyph>&                                  all_glyphs,
    const QList<qsizetype>&                              glyphs_to_pack,
    QSize                                                bin_size,
    binpacking::MaxRectsBinPack::FreeRectChoiceHeuristic heuristic)
{
    auto binPack = binpacking::MaxRectsBinPack(bin_size);

    for (int i = int(glyphs_to_pack.size()) - 1; i >= 0; --i)
    {
        const Glyph& glyph = all_glyphs[glyphs_to_pack[i]];

        if (glyph.rect.width() == 0 && glyph.rect.height() == 0)
        {
            continue;
        }

        if (const auto result_rect = binPack.insert(glyph.rect.size(), heuristic);
            result_rect.width() == 0 && result_rect.height() == 0)
        {
            return {false, binPack.occupancy()};
        }
    }

    return {true, 1.0F};
}

qsizetype FontGenContext::insert_as_many_glyphs_as_possible(
    QList<Glyph>&                                        all_glyphs,
    QList<qsizetype>&                                    glyphs_to_insert,
    QSize                                                bin_size,
    binpacking::MaxRectsBinPack::FreeRectChoiceHeuristic heuristic,
    QList<qsizetype>&                                    destination) const
{
    int num_inserted_glyphs{};

    auto bin_pack = binpacking::MaxRectsBinPack(bin_size);

    while (!glyphs_to_insert.empty())
    {
        if (m_is_canceled)
        {
            return 0;
        }

        const auto glyph_idx = glyphs_to_insert.back();
        Glyph&     glyph     = all_glyphs[glyph_idx];

        if (glyph.rect.width() == 0 && glyph.rect.height() == 0)
        {
            destination.push_back(glyph_idx);
            glyphs_to_insert.pop_back();
            ++num_inserted_glyphs;
            continue;
        }

        const auto result_rect = bin_pack.insert(glyph.rect.size(), heuristic);

        if (result_rect.width() == 0 && result_rect.height() == 0)
        {
            break;
        }

        glyph.rect = result_rect;
        destination.push_back(glyph_idx);
        glyphs_to_insert.pop_back();
        ++num_inserted_glyphs;
    }

    return num_inserted_glyphs;
}

std::optional<QList<FontPage>> FontGenContext::pack_glyphs(QList<Glyph>& glyphs, int max_page_size)
{
    QList<qsizetype> glyphs_to_insert;
    glyphs_to_insert.reserve(glyphs.size());

    for (qsizetype i = 0; i < glyphs.size(); ++i)
    {
        glyphs_to_insert.push_back(i);
    }

    auto bin_size = QSize(32, 32);

    using Heuristic = binpacking::MaxRectsBinPack::FreeRectChoiceHeuristic;

    constexpr std::array heuristics = {
        Heuristic::RectBestShortSideFit,
        Heuristic::RectBestLongSideFit,
        Heuristic::RectBestAreaFit,
        Heuristic::RectBottomLeftRule,
    };

    auto pages = QList<FontPage>();

    while (!glyphs_to_insert.empty())
    {
        if (m_is_canceled)
        {
            return {};
        }

        auto occupancies = std::array<float, heuristics.size()>();

        auto index = 0;
        for (auto heuristic : heuristics)
        {
            const auto [canFitAll, occupancy] =
                can_fit_all_glyphs(glyphs, glyphs_to_insert, bin_size, heuristic);

            occupancies[index] = occupancy;

            if (canFitAll)
            {
                pages.emplace_back(bin_size);
                insert_as_many_glyphs_as_possible(glyphs,
                                                  glyphs_to_insert,
                                                  bin_size,
                                                  heuristic,
                                                  pages.back().glyph_indices);

                if (!m_is_canceled)
                {
                    Q_ASSERT(glyphs_to_insert.empty());
                }
                else
                {
                    return {};
                }

                break;
            }

            ++index;
        }

        if (glyphs_to_insert.empty())
        {
            break;
        }

        // Try doubling the texture size.
        bin_size *= 2;

        if (bin_size.width() > max_page_size || bin_size.height() > max_page_size)
        {
            // Okay, we have exceeded the texture size limit. We have to pack
            // as many glyphs as possible into the current page and proceed
            // with a new page.
            bin_size /= 2;

            pages.emplace_back(bin_size);

            const int best_heuristic_index = int(
                std::distance(std::begin(occupancies),
                              std::max_element(std::begin(occupancies), std::end(occupancies))));

            const auto heuristic = heuristics[best_heuristic_index];

            const qsizetype num_inserted_glyphs =
                insert_as_many_glyphs_as_possible(glyphs,
                                                  glyphs_to_insert,
                                                  bin_size,
                                                  heuristic,
                                                  pages.back().glyph_indices);

            if (num_inserted_glyphs == 0)
            {
                return std::optional<QList<FontPage>>();
            }

            bin_size = QSize(32, 32);
        }
    }

    for (int p = 0; p < pages.size(); ++p)
    {
        const FontPage& page = pages[p];

        for (const qsizetype glyph_index : page.glyph_indices)
        {
            Glyph& glyph     = glyphs[glyph_index];
            glyph.page_index = p;
        }
    }

    return pages;
}

static void MoveGlyphDataToTheirPages(const QList<Glyph>& all_glyphs, QList<FontPage>& pages)
{
    for (FontPage& page : pages)
    {
        QPainter painter{&page.image};

        for (const qsizetype glyph_index : page.glyph_indices)
        {
            const auto& glyph = all_glyphs[glyph_index];
            painter.drawImage(glyph.rect.topLeft(), glyph.image);
        }
    }
}

std::optional<QList<FontPage>> FontGenContext::create_font_pages(QList<Glyph>& glyphs,
                                                                 int           max_page_extent)
{
    auto maybe_pages = pack_glyphs(glyphs, max_page_extent);

    if (!maybe_pages.has_value())
    {
        return {};
    }

    auto pages = std::move(*maybe_pages);

    MoveGlyphDataToTheirPages(glyphs, pages);

    return pages;
}

static QPointF rotate_around_center(const QPointF& pt, const QPointF& center, float degrees)
{
    const float radians = qDegreesToRadians(degrees);

    const float cos = std::cos(radians);
    const float sin = std::sin(radians);

    const QPointF ret = pt - center;

    const qreal xnew = ret.x() * cos - ret.y() * sin;
    const qreal ynew = ret.x() * sin + ret.y() * cos;

    return QPointF{xnew + center.x(), ynew + center.y()};
}

static QBrush get_brush_for_fill(ImageCache&      image_cache,
                                 const FontModel& font,
                                 const FontFill&  fill,
                                 QSize            size)
{
    if (fill.fill_type == FillType::None)
    {
        return Qt::transparent;
    }

    if (fill.fill_type == FillType::SolidColor)
    {
        return fill.solid_color;
    }

    if (fill.fill_type == FillType::LinearGradient)
    {
        const auto anglef = float(fill.gradient_angle + 90);

        const QPointF start = rotate_around_center(QPointF(-0.25, 0.5), QPointF(0.5, 0.5), anglef);
        const QPointF end   = rotate_around_center(QPointF(1.25, 0.5), QPointF(0.5, 0.5), anglef);

        QLinearGradient grad;
        grad.setStops(fill.gradient_stops);
        grad.setCoordinateMode(QGradient::ObjectMode);
        grad.setStart(start);
        grad.setFinalStop(end);

        return QBrush{grad};
    }

    if (fill.fill_type == FillType::RadialGradient)
    {
        const qreal xoffset = std::lerp(0U, size.width(), qreal(fill.gradient_offset.x() * 0.01));
        const qreal yoffset = std::lerp(0U, size.height(), qreal(fill.gradient_offset.y() * 0.01));
        const qreal radius  = std::lerp(0U,
                                       std::max(size.width(), size.height()),
                                       qreal(fill.gradient_radius * 0.01));

        QRadialGradient grad{QPointF(xoffset, yoffset), radius};
        grad.setStops(fill.gradient_stops);

        return QBrush{grad};
    }

    if (fill.fill_type == FillType::Image)
    {
        const QImage img = image_cache.lookup(font.absolute_filename(fill.image_filename));

        return img.scaled(size);
    }

    return {};
}


std::shared_ptr<GeneratedFont> FontGenContext::generate_bitmap_font(
    const FontModel& font, std::optional<QSet<QChar>> characters_override)
{
    const auto characters = characters_override ? *characters_override : font.characters();

    const FontFill base_fill   = font.base_fill();
    const FontFill stroke_fill = font.stroke_fill();

    const bool is_outlined = [&] {
        if (stroke_fill.fill_type == FillType::None)
        {
            return false;
        }

        if (stroke_fill.fill_type == FillType::SolidColor && stroke_fill.solid_color.alpha() == 0)
        {
            return false;
        }

        return true;
    }();

    const int outline_width = is_outlined ? font.stroke_spread() : 0;

    QList<GeneratedFont::Variation> variations;
    variations.reserve(font.variations().size());

    QList<Glyph> all_glyphs;
    all_glyphs.reserve(characters.size() * font.variations().size());

    for (const auto& variation : font.variations())
    {
        Q_ASSERT(variation >= 0.5);

        QList<qsizetype> glyph_indices;
        glyph_indices.reserve(characters.size());

        QFont qfont = font.qfont();

        int size = qfont.pixelSize();
        if (size <= 0)
        {
            size = qfont.pointSize();
        }

        if (size <= 0)
        {
            throw InvalidFontFaceError();
        }

        const int variation_size = static_cast<int>(static_cast<double>(size) * variation);

        Q_ASSERT(variation_size > 0);

        qfont.setPixelSize(variation_size);
        qfont.setKerning(font.use_kerning());

        QFontMetrics font_metrics{qfont};

        for (const auto ch : characters)
        {
            if (!font_metrics.inFont(ch))
            {
                continue;
            }

            QString chh_str;
            chh_str += ch;

            auto glyphSize = font_metrics.size(Qt::TextSingleLine, ch);

            QSize extra_size;
            extra_size.setWidth(outline_width * 2);
            extra_size.setHeight(outline_width * 2);
            glyphSize += extra_size;

            Glyph glyph{
                .character          = ch,
                .rect               = QRect{0, 0, glyphSize.width(), glyphSize.height()},
                .page_index         = 0,
                .horizontal_advance = font_metrics.horizontalAdvance(ch),
                .left_bearing       = font_metrics.leftBearing(ch),
                .right_bearing      = font_metrics.rightBearing(ch),
                .image = QImage{glyphSize.width(), glyphSize.height(), QImage::Format_RGBA8888},
            };

            glyph.image.fill(Qt::transparent);

            QPainter painter{&glyph.image};
            painter.setRenderHint(QPainter::TextAntialiasing, font.anti_aliasing());
            painter.setRenderHint(QPainter::Antialiasing, font.anti_aliasing());
            painter.setFont(qfont);
            painter.setPen(Qt::white);

            // Draw
            const int text_pos_x = (extra_size.width()) / 2;
            const int text_pos_y = (extra_size.height() / 2) + font_metrics.ascent();

            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addText(text_pos_x, text_pos_y, qfont, chh_str);

            if (is_outlined)
            {
                QPen pen;
                pen.setBrush(
                    get_brush_for_fill(m_image_cache, font, font.stroke_fill(), glyphSize));
                pen.setWidthF(outline_width);
                pen.setStyle(font.stroke_style());
                pen.setCapStyle(font.stroke_cap_style());
                pen.setJoinStyle(font.stroke_join_style());

                if (font.stroke_join_style() == Qt::MiterJoin)
                {
                    pen.setMiterLimit(font.stroke_miter_limit() * 0.01);
                }

                if (font.stroke_style() != Qt::SolidLine)
                {
                    pen.setDashOffset(font.stroke_dash_offset() * 0.05);
                }

                painter.strokePath(path, pen);
            }

            const QBrush fill_brush =
                get_brush_for_fill(m_image_cache, font, font.base_fill(), glyphSize);

            painter.fillPath(path, fill_brush);

            glyph_indices.push_back(all_glyphs.size());
            all_glyphs.push_back(std::move(glyph));
        }

        variations.push_back(GeneratedFont::Variation{
            .scale_factor       = variation,
            .line_height        = font_metrics.height(),
            .ascent             = font_metrics.ascent(),
            .descent            = font_metrics.descent(),
            .line_gap           = font_metrics.lineSpacing(),
            .underline_position = font_metrics.underlinePos(),
            .glyph_indices      = std::move(glyph_indices),
        });
    }

    auto maybe_pages = create_font_pages(all_glyphs, font.max_page_extent());

    if (!maybe_pages)
    {
        throw GlyphsDontFitError();
    }

    return std::make_shared<GeneratedFont>(font.name(),
                                           font.qfont().pixelSize(),
                                           characters,
                                           std::move(all_glyphs),
                                           std::move(variations),
                                           std::move(*maybe_pages));
}

FontGenContext::FontGenContext(QSet<QChar>* all_characters)
    : m_all_characters(all_characters)
{
}

std::shared_ptr<GeneratedFont> FontGenContext::generate_font(
    const FontModel& model, const std::optional<QSet<QChar>>& characters_override)
{
    m_image_cache.clear();

    return generate_bitmap_font(model, characters_override);
}

void FontGenContext::cancel()
{
    m_is_canceled = true;
}
