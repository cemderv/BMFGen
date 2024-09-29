// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "GeneratedFont.hpp"

#include "QtImageUtil.hpp"
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QXmlStreamWriter>
#include <algorithm>
#include <fstream>
#include <ostream>
#include <stdexcept>

FontExportImageType font_export_image_type_from_string(const QString& value)
{
    if (value == "png")
    {
        return FontExportImageType::Png;
    }

    if (value == "bmp")
    {
        return FontExportImageType::Bmp;
    }

    return FontExportImageType::Png;
}

QString font_export_image_type_to_string(FontExportImageType type)
{
    switch (type)
    {
        case FontExportImageType::Png: return QStringLiteral("PNG");
        case FontExportImageType::Bmp: return QStringLiteral("BMP");
    }

    return QStringLiteral("PNG");
}


GeneratedFont::GeneratedFont(QString          name,
                             int              base_size,
                             QSet<QChar>      characters,
                             QList<Glyph>     all_glyphs,
                             QList<Variation> variations,
                             QList<FontPage>  pages)
    : m_name(std::move(name))
    , m_base_size(base_size)
    , m_chars(std::move(characters))
    , m_all_glyphs(std::move(all_glyphs))
    , m_variations(std::move(variations))
    , m_pages(std::move(pages))
{
    for (Variation& var : m_variations)
    {
        var.parent_font = this;
    }
}

bool GeneratedFont::has_char(QChar character) const
{
    return m_chars.contains(character);
}

int GeneratedFont::base_size() const
{
    return m_base_size;
}

const QList<Glyph>& GeneratedFont::all_glyphs() const
{
    return m_all_glyphs;
}

const Glyph* GeneratedFont::Variation::find_glyph(QChar ch) const
{
    const auto& all_glyphs = parent_font->all_glyphs();

    for (const auto& glyph_index : glyph_indices)
    {
        const auto& glyph = all_glyphs[glyph_index];
        if (glyph.character == ch)
        {
            return &glyph;
        }
    }

    return nullptr;
}

int GeneratedFont::Variation::pixel_size() const
{
    return static_cast<int>(static_cast<double>(parent_font->base_size()) * scale_factor);
}

const QList<FontPage>& GeneratedFont::pages() const
{
    return m_pages;
}

const FontPage& GeneratedFont::page_at(int index) const
{
    return m_pages[index];
}

void GeneratedFont::export_to_disk(const QString&      directory,
                                   FontDescriptionType font_description_type,
                                   FontExportImageType image_type,
                                   bool                flip_images_upside_down,
                                   bool                allow_monochromatic_images) const
{
    qDebug("Exporting font to disk: %s", qPrintable(directory));

    if (!QDir{directory}.mkpath("."))
    {
        throw std::runtime_error{
            QStringLiteral("Failed to create directory '%1'").arg(directory).toStdString()};
    }

    std::optional<QStringList> images_filenames =
        export_images(directory, image_type, flip_images_upside_down, allow_monochromatic_images);

    if (!images_filenames.has_value())
    {
        throw std::runtime_error{"Failed to export the font images."};
    }

    const ExportArgs args{
        .directory        = directory,
        .images_filenames = *images_filenames,
    };

    switch (font_description_type)
    {
        case FontDescriptionType::JSON: export_as_json(args); break;
        case FontDescriptionType::XML: export_as_xml(args); break;
        case FontDescriptionType::Text: export_as_text(args); break;
    }
}

qsizetype GeneratedFont::size_in_bytes(bool allow_monochromatic_images) const
{
    qsizetype sum = 0;

    for (const auto& page : m_pages)
    {
        int byte_stride = 4; // 32-bit RGBA

        if (allow_monochromatic_images && QtImageUtil::is_monochromatic(page.image))
        {
            byte_stride = 1;
        }

        sum += page.image.width() * page.image.height() * byte_stride;
    }

    return sum;
}

qsizetype GeneratedFont::total_glyph_count() const
{
    qsizetype sum = 0;

    for (const auto& var : m_variations)
    {
        sum += var.glyph_indices.size();
    }

    return sum;
}

int GeneratedFont::variation_count() const
{
    return static_cast<int>(m_variations.size());
}

const GeneratedFont::Variation& GeneratedFont::default_variation() const
{
    return variation_by_scale(1.0);
}

const GeneratedFont::Variation& GeneratedFont::variation_at(int index) const
{
    return m_variations.at(index);
}


const GeneratedFont::Variation& GeneratedFont::variation_by_scale(double scale) const
{
    for (const auto& var : m_variations)
    {
        if (var.scale_factor == scale)
        {
            return var;
        }
    }

    return m_variations.first();
}

static inline void write(std::ostream& ofs, int32_t value)
{
    ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

static inline void write(std::ostream& ofs, uint32_t value)
{
    ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

static inline void write(std::ostream& ofs, uint64_t value)
{
    ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

static inline void write(std::ostream& ofs, const QString& value_)
{
    const std::string value = value_.toStdString();
    write(ofs, uint32_t(value.length()));
    ofs.write(value.c_str(), value.length());
}

std::optional<QStringList> GeneratedFont::export_images(const QString&      directory,
                                                        FontExportImageType type,
                                                        bool                flip_upside_down,
                                                        bool allow_monochromatic) const
{
    QStringList filenames;

    int index{};

    for (const auto& page : m_pages)
    {
        const char* extension = [type]() {
            switch (type)
            {
                case FontExportImageType::Png: return ".png";
                case FontExportImageType::Bmp: return ".bmp";
            }

            return "";
        }();

        const QString filename =
            QDir::cleanPath(directory + QDir::separator() +
                            QStringLiteral("%1_%2%3").arg(m_name).arg(index).arg(extension));

        QImage image = flip_upside_down ? page.image.mirrored(false, true) : page.image;

        if (image.isNull())
        {
            qCritical("Failed to save image to: %s", qPrintable(filename));
            return std::nullopt;
        }

        if (allow_monochromatic && QtImageUtil::is_monochromatic(image))
        {
            image.convertTo(QImage::Format_Grayscale8);
        }

        if (!image.save(filename))
        {
            return std::nullopt;
        }

        filenames.append(filename);

        ++index;
    }

    return filenames;
}

void GeneratedFont::export_as_json(const ExportArgs& args) const
{
    QJsonObject root_obj;

    root_obj.insert("name", m_name);
    root_obj.insert("baseSize", m_base_size);

    // Pages
    {
        root_obj.insert("pageCount", m_pages.size());

        QJsonArray pages_array;
        qsizetype  index = 0;

        for (const auto& page : m_pages)
        {
            QJsonObject page_obj;
            page_obj.insert("index", index);
            page_obj.insert("filename", QFileInfo{args.images_filenames.at(index)}.fileName());
            page_obj.insert("width", page.image.width());
            page_obj.insert("height", page.image.height());

            pages_array.push_back(page_obj);

            ++index;
        }

        root_obj.insert("pages", pages_array);
    }

    // Glyphs
    {
        root_obj.insert("glyphCount", m_all_glyphs.size());

        QJsonArray glyphs_arr;
        qsizetype  index = 0;

        for (const auto& glyph : m_all_glyphs)
        {
            QJsonObject glyphObj;
            glyphObj.insert("index", index);
            glyphObj.insert("character", QJsonValue::fromVariant(glyph.character));
            glyphObj.insert("x", glyph.rect.x());
            glyphObj.insert("y", glyph.rect.y());
            glyphObj.insert("width", glyph.rect.width());
            glyphObj.insert("height", glyph.rect.height());
            glyphObj.insert("pageIndex", glyph.page_index);
            glyphObj.insert("horizontalAdvance", glyph.horizontal_advance);
            glyphObj.insert("leftBearing", glyph.left_bearing);
            glyphObj.insert("rightBearing", glyph.right_bearing);

            glyphs_arr.push_back(glyphObj);

            ++index;
        }

        root_obj.insert("glyphs", glyphs_arr);
    }

    // Variations
    {
        root_obj.insert("variationCount", m_variations.size());

        QJsonArray variations_json_arr;

        qsizetype index = 0;

        for (const auto& variation : m_variations)
        {
            QJsonObject obj;
            obj.insert("index", index);
            obj.insert("scaleFactor", variation.scale_factor);
            obj.insert("pixelSize", variation.pixel_size());
            obj.insert("lineHeight", variation.line_height);
            obj.insert("ascent", variation.ascent);
            obj.insert("descent", variation.descent);
            obj.insert("lineGap", variation.line_gap);
            obj.insert("underlinePosition", variation.underline_position);

            QJsonArray glyph_indices_arr;

            for (const auto glyph_index : variation.glyph_indices)
            {
                glyph_indices_arr.append(QJsonValue::fromVariant(int(glyph_index)));
            }

            obj.insert("glyphIndices", glyph_indices_arr);

            variations_json_arr.append(obj);

            ++index;
        }

        root_obj.insert("variations", variations_json_arr);
    }

    const QString filename = QDir::cleanPath(args.directory + QDir::separator() + m_name + ".json");

    QFile file{filename};

    if (!file.open(QFile::WriteOnly))
    {
        throw std::runtime_error(
            QStringLiteral("Failed to open file '%1' for writing.").arg(filename).toStdString());
    }

    file.write(QJsonDocument{root_obj}.toJson());
}

void GeneratedFont::export_as_xml(const ExportArgs& args) const
{
    const QString filename = QDir::cleanPath(args.directory + QDir::separator() + m_name + ".xml");

    QFile file{filename};

    if (!file.open(QFile::WriteOnly))
    {
        throw std::runtime_error(
            QStringLiteral("Failed to open file '%1' for writing.").arg(filename).toStdString());
    }

    QXmlStreamWriter stream{&file};
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement("font");
    stream.writeTextElement("name", m_name);
    stream.writeTextElement("baseSize", QString::number(m_base_size));

    // Pages
    {
        stream.writeTextElement("pageCount", QString::number(m_pages.size()));

        stream.writeStartElement("pages");

        qsizetype index = 0;
        for (const auto& page : pages())
        {
            stream.writeStartElement("page");
            stream.writeTextElement("index", QString::number(index));
            stream.writeTextElement("filename",
                                    QFileInfo{args.images_filenames.at(index)}.fileName());
            stream.writeTextElement("width", QString::number(page.image.width()));
            stream.writeTextElement("height", QString::number(page.image.height()));
            stream.writeEndElement();
            ++index;
        }

        stream.writeEndElement(); // pages
    }

    // Glyphs
    {
        stream.writeTextElement("glyphCount", QString::number(m_all_glyphs.size()));
        stream.writeStartElement("glyphs");

        qsizetype index = 0;

        for (const auto& glyph : m_all_glyphs)
        {
            stream.writeStartElement("glyph");

            stream.writeTextElement("index", QString::number(index));
            stream.writeTextElement("character", glyph.character);
            stream.writeTextElement("x", QString::number(glyph.rect.x()));
            stream.writeTextElement("y", QString::number(glyph.rect.y()));
            stream.writeTextElement("width", QString::number(glyph.rect.width()));
            stream.writeTextElement("height", QString::number(glyph.rect.height()));
            stream.writeTextElement("pageIndex", QString::number(glyph.page_index));
            stream.writeTextElement("horizontalAdvance", QString::number(glyph.horizontal_advance));
            stream.writeTextElement("leftBearing", QString::number(glyph.left_bearing));
            stream.writeTextElement("rightBearing", QString::number(glyph.right_bearing));

            stream.writeEndElement(); // glyph

            ++index;
        }

        stream.writeEndElement(); // glyphs
    }

    // Variations
    {
        stream.writeTextElement("variationCount", QString::number(m_variations.size()));
        stream.writeStartElement("variations");

        qsizetype index = 0;

        for (const auto& variation : m_variations)
        {
            stream.writeStartElement("variation");
            stream.writeTextElement("index", QString::number(index));
            stream.writeTextElement("scaleFactor", QString::number(variation.scale_factor));
            stream.writeTextElement("pixelSize", QString::number(variation.pixel_size()));
            stream.writeTextElement("lineHeight", QString::number(variation.line_height));
            stream.writeTextElement("ascent", QString::number(variation.ascent));
            stream.writeTextElement("descent", QString::number(variation.descent));
            stream.writeTextElement("lineGap", QString::number(variation.line_gap));
            stream.writeTextElement("underlinePosition",
                                    QString::number(variation.underline_position));

            stream.writeStartElement("glyphIndices");
            for (const auto glyph_index : variation.glyph_indices)
            {
                stream.writeStartElement("index");
                stream.writeAttribute("value", QString::number(glyph_index));
                stream.writeEndElement();
            }
            stream.writeEndElement();

            stream.writeEndElement();

            ++index;
        }

        stream.writeEndElement();
    }

    stream.writeEndElement(); // font

    stream.writeEndDocument();
}

void GeneratedFont::export_as_text(const ExportArgs& args) const
{
    const QString filename = QDir::cleanPath(args.directory + QDir::separator() + m_name + ".txt");

    QFile file{filename};

    if (!file.open(QFile::WriteOnly))
    {
        throw std::runtime_error(
            QStringLiteral("Failed to open file '%1' for writing.").arg(filename).toStdString());
    }

    constexpr char nl = '\n';

    QTextStream w{&file};

    w << "name " << m_name << nl;
    w << "baseSize " << m_base_size << nl;

    // Pages
    {
        w << "pageCount " << m_pages.size() << nl;
        w << "pages" << nl;

        qsizetype index = 0;
        for (const auto& page : m_pages)
        {
            w << "page" << nl;
            w << "index " << index << nl;
            w << "filename " << QFileInfo{args.images_filenames.at(index)}.fileName() << nl;
            w << "width " << page.image.width() << nl;
            w << "height " << page.image.height() << nl;
            w << "end" << nl;
            ++index;
        }

        w << "end" << nl;
    }

    // Glyphs
    {
        w << "glyphCount " << m_all_glyphs.size() << nl;
        w << "glyphs" << nl;

        qsizetype index = 0;

        for (const auto& glyph : m_all_glyphs)
        {
            w << "glyph" << nl;

            w << "index " << index << nl;
            w << "character " << glyph.character << nl;
            w << "x " << glyph.rect.x() << nl;
            w << "y " << glyph.rect.y() << nl;
            w << "width " << glyph.rect.width() << nl;
            w << "height " << glyph.rect.height() << nl;
            w << "pageIndex " << glyph.page_index << nl;
            w << "horizontalAdvance " << glyph.horizontal_advance << nl;
            w << "leftBearing " << glyph.left_bearing << nl;
            w << "rightBearing " << glyph.right_bearing << nl;

            w << "end" << nl;

            ++index;
        }

        w << "end" << nl;
    }

    // Variations
    {
        w << "variationCount " << m_variations.size() << nl;
        w << "variations" << nl;

        qsizetype index = 0;

        for (const auto& variation : m_variations)
        {
            w << "variation" << nl;

            w << "index " << index << nl;
            w << "scaleFactor " << variation.scale_factor << nl;
            w << "pixelSize " << variation.pixel_size() << nl;
            w << "lineHeight " << variation.line_height << nl;
            w << "ascent " << variation.ascent << nl;
            w << "descent " << variation.descent << nl;
            w << "lineGap " << variation.line_gap << nl;
            w << "underlinePosition " << variation.underline_position << nl;

            w << "glyphIndices";
            for (const auto& glyph_index : variation.glyph_indices)
            {
                w << " " << glyph_index;
            }
            w << nl;

            w << "end" << nl;

            ++index;
        }
    }

    w << nl;
}

inline void hash_combine(std::size_t&)
{
}

template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, Rest... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}

uint64_t GeneratedFont::build_cache_key() const
{
    std::size_t seed{};
    hash_combine(seed,
                 qHash(m_name, 0),
                 m_base_size,
                 m_chars.size(),
                 m_all_glyphs.size(),
                 m_variations.size(),
                 m_pages.size());

    for (const auto& var : m_variations)
    {
        hash_combine(seed,
                     var.scale_factor,
                     var.line_height,
                     var.ascent,
                     var.descent,
                     var.line_gap,
                     var.underline_position,
                     var.glyph_indices.size());
    }

    for (const auto& page : m_pages)
    {
        hash_combine(seed, page.image.width(), page.image.height(), page.glyph_indices.size());
    }

    return seed;
}
