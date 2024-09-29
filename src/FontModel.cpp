// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontModel.hpp"

#include "QtJsonUtil.hpp"
#include <QDir>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

static const QString s_default_export_directory = QStringLiteral("$(FONT_NAME)_exported/");

static QJsonObject FontFillToJson(const FontFill& fill)
{
    auto obj = QJsonObject();

    obj.insert(QStringLiteral("fill_type"), [type = fill.fill_type] {
        switch (type)
        {
            case FillType::None: return QStringLiteral("none");
            case FillType::SolidColor: return QStringLiteral("solid_color");
            case FillType::LinearGradient: return QStringLiteral("linear_gradient");
            case FillType::RadialGradient: return QStringLiteral("radial_gradient");
            case FillType::Image: return QStringLiteral("image");
        }

        return QString{};
    }());

    obj.insert(QStringLiteral("solid_color"), color_to_json(fill.solid_color));
    obj.insert(QStringLiteral("gradient_stops"), gradient_stops_to_json(fill.gradient_stops));
    obj.insert(QStringLiteral("gradient_angle"), fill.gradient_angle);
    obj.insert(QStringLiteral("gradient_offset_x"), fill.gradient_offset.x());
    obj.insert(QStringLiteral("gradient_offset_y"), fill.gradient_offset.y());
    obj.insert(QStringLiteral("gradient_radius"), fill.gradient_radius);
    obj.insert(QStringLiteral("image"), fill.image_filename);

    return obj;
}

static FillType parse_fill_type(const QString& str)
{
    if (str == "none")
    {
        return FillType::None;
    }
    if (str == "solid_color")
    {
        return FillType::SolidColor;
    }
    if (str == "linear_gradient")
    {
        return FillType::LinearGradient;
    }
    if (str == "radial_gradient")
    {
        return FillType::RadialGradient;
    }
    if (str == "image")
    {
        return FillType::Image;
    }

    return FillType::SolidColor;
}

static std::pair<QString, FontFill> get_json_fill(const QJsonObject& json_value,
                                                  const QString&     name)
{
    const QJsonValue& value = json_value[name];

    if (!value.isObject())
    {
        return {QObject::tr("Property '%s' must be an object.").arg(name), FontFill{}};
    }

    const QJsonObject& json_fill_obj = value.toObject();

    const QString fill_type_str =
        get_json_string(json_fill_obj, QStringLiteral("fill_type")).value_or(QString{});

    const FillType type = parse_fill_type(fill_type_str);

    const auto solid_color =
        get_json_color(json_fill_obj, QStringLiteral("solid_color")).value_or(Qt::white);

    const QGradientStops default_stops{
        {0.0, Qt::black},
        {1.0, Qt::white},
    };

    const QGradientStops gradient_stops =
        get_json_gradient_stops(json_fill_obj, QStringLiteral("gradient_stops"))
            .value_or(default_stops);

    const int gradient_angle =
        get_json_int(json_fill_obj, QStringLiteral("gradient_angle")).value_or(0);

    const int gradient_offset_x =
        get_json_int(json_fill_obj, QStringLiteral("gradient_offset_x")).value_or(0);

    const int gradient_offset_y =
        get_json_int(json_fill_obj, QStringLiteral("gradient_offset_y")).value_or(0);

    const int gradient_radius =
        std::clamp(get_json_int(json_fill_obj, QStringLiteral("gradient_radius")).value_or(0),
                   0,
                   100);

    const QString image_filename =
        get_json_string(json_fill_obj, QStringLiteral("image")).value_or(QString{});

    return {QString{},
            FontFill{
                .fill_type       = type,
                .solid_color     = solid_color,
                .gradient_stops  = gradient_stops,
                .gradient_angle  = gradient_angle,
                .gradient_offset = {gradient_offset_x, gradient_offset_y},
                .gradient_radius = gradient_radius,
                .image_filename  = image_filename,
            }};
}

FontModel::FontModel(QObject* parent, const QString& filename)
    : QObject(parent)
    , m_filename(filename)
{
    if (!load_from_file(filename))
    {
        return;
    }
}

FontModel::~FontModel() = default;

std::optional<QString> FontModel::create_new_font_file_at(const QString& filename)
{
    // Load default config from storage and replace placeholders.
    QString file_contents;
    {
        QFile file{QStringLiteral(":/DefaultFontConfig.json")};
        if (!file.open(QFile::ReadOnly))
        {
            return tr("Failed to load the default font configuration.");
        }

        file_contents = file.readAll();
    }

    file_contents.replace("\"${FONT_SIZE}\"", QStringLiteral("48"));
    file_contents.replace("\"${MAX_PAGE_EXTENT}\"", QStringLiteral("1024"));

    // Write the new file.
    {
        QFile file{filename};
        if (!file.open(QFile::WriteOnly | QFile::Text))
        {
            return tr("Failed to open file '%1' for writing.").arg(filename);
        }

        QTextStream stream{&file};
        stream << file_contents;
    }

    return {};
}

QString FontModel::error_message() const
{
    return m_error_message;
}

QString FontModel::name() const
{
    return QFileInfo{m_filename}.baseName();
}

QString FontModel::filename() const
{
    return m_filename;
}

std::optional<QString> FontModel::save_to_file(const QString& filename)
{
    QJsonObject root_obj;

    root_obj.insert(QStringLiteral("name"), name());

    {
        root_obj.insert(QStringLiteral("font_family"), m_qfont.family());
        root_obj.insert(QStringLiteral("is_bold"), m_qfont.bold());
        root_obj.insert(QStringLiteral("is_italic"), m_qfont.italic());
        root_obj.insert(QStringLiteral("font_pixel_size"), m_qfont.pixelSize());
    }

    root_obj.insert(QStringLiteral("max_page_extent"), m_max_page_extent);
    root_obj.insert("anti_aliasing", m_anti_aliasing);

    root_obj.insert(QStringLiteral("desc_type"), [this] {
        switch (m_desc_type)
        {
            case FontDescriptionType::JSON: return QStringLiteral("json");
            case FontDescriptionType::XML: return QStringLiteral("xml");
            case FontDescriptionType::Text: return QStringLiteral("text");
        }
        return QStringLiteral("binary");
    }());

    root_obj.insert(QStringLiteral("image_type"), font_export_image_type_to_string(m_image_type));
    root_obj.insert(QStringLiteral("export_directory"), m_export_directory);
    root_obj.insert(QStringLiteral("flip_images_upside_down"), m_should_flip_images_upside_down);

    root_obj.insert(QStringLiteral("preview_background_color"),
                    color_to_json(m_preview_background_color));

    root_obj.insert(QStringLiteral("base_fill"), FontFillToJson(m_base_fill));
    root_obj.insert(QStringLiteral("stroke_fill"), FontFillToJson(m_stroke_fill));

    root_obj.insert(QStringLiteral("stroke_spread"), m_stroke_spread);

    root_obj.insert(QStringLiteral("stroke_style"), [this] {
        switch (m_stroke_style)
        {
            case Qt::PenStyle::SolidLine: return QStringLiteral("solid_line");
            case Qt::PenStyle::DashLine: return QStringLiteral("dash_line");
            case Qt::PenStyle::DotLine: return QStringLiteral("dot_line");
            case Qt::PenStyle::DashDotLine: return QStringLiteral("dash_dot_line");
            case Qt::PenStyle::DashDotDotLine: return QStringLiteral("dash_dot_dot_line");
            case Qt::PenStyle::NoPen:
            case Qt::PenStyle::CustomDashLine:
            case Qt::PenStyle::MPenStyle: return QString{};
        }

        return QString{};
    }());

    root_obj.insert(QStringLiteral("stroke_cap_style"), [this] {
        switch (m_stroke_cap_style)
        {
            case Qt::FlatCap: return QStringLiteral("flat");
            case Qt::SquareCap: return QStringLiteral("square");
            case Qt::RoundCap: return QStringLiteral("round");
            default: return QString{};
        }

        return QStringLiteral("Flat");
    }());

    root_obj.insert(QStringLiteral("stroke_join_style"), [this] {
        switch (m_stroke_join_style)
        {
            case Qt::MiterJoin: return QStringLiteral("miter");
            case Qt::BevelJoin: return QStringLiteral("bevel");
            case Qt::RoundJoin: return QStringLiteral("round");
            case Qt::SvgMiterJoin:
            case Qt::MPenJoinStyle: return QString{};
        }

        return QString{};
    }());

    root_obj.insert(QStringLiteral("stroke_miter_limit"), m_stroke_miter_limit);
    root_obj.insert(QStringLiteral("stroke_dash_offset"), m_stroke_dash_offset);
    root_obj.insert(QStringLiteral("chars"), char_set_to_json(m_characters));
    root_obj.insert(QStringLiteral("variations"), double_set_to_json(m_variations));

    QJsonDocument doc;
    doc.setObject(root_obj);

    {
        QFile file{filename};
        if (!file.open(QFile::WriteOnly))
        {
            return tr("Failed to open file '%1' for writing.").arg(filename);
        }

        QTextStream stream{&file};
        stream << doc.toJson();
    }

    m_filename = filename;

    return {};
}

GeneratedFont* FontModel::generated_font() const
{
    return m_generated_font.get();
}

void FontModel::set_generated_font(std::shared_ptr<GeneratedFont> font)
{
    m_generated_font = std::move(font);
    emit generated_font_changed();
}

QColor FontModel::preview_background_color() const
{
    return m_preview_background_color;
}

void FontModel::set_preview_background_color(const QColor& value)
{
    if (m_preview_background_color != value)
    {
        m_preview_background_color = value;
        emit properties_only_relevant_for_save_changed();
    }
}

QString FontModel::preview_text() const
{
    return m_preview_text;
}

void FontModel::set_preview_text(const QString& value)
{
    if (m_preview_text != value)
    {
        m_preview_text = value;
        emit properties_only_relevant_for_save_changed();
    }
}

static Qt::PenStyle parse_stroke_style(const QString& s)
{
    if (s == "SolidLine")
        return Qt::SolidLine;
    if (s == "DashLine")
        return Qt::DashLine;
    if (s == "DotLine")
        return Qt::DotLine;
    if (s == "DashDotLine")
        return Qt::DashDotLine;
    if (s == "DashDotDotLine")
        return Qt::DashDotDotLine;

    return Qt::SolidLine;
}

std::optional<QString> FontModel::load_from_file(const QString& filename)
{
    QJsonDocument doc;
    {
        QFile file{filename};
        if (!file.open(QFile::ReadOnly))
        {
            return tr("Failed to open file '%1' for reading.").arg(filename);
        }

        doc = QJsonDocument::fromJson(file.readAll());
    }

    if (doc.isNull())
    {
        return tr("File '%s' does not represent a valid JSON file.").arg(filename);
    }

    const auto& obj = doc.object();

    if (obj.isEmpty())
    {
        return tr("File '%s' does not represent a JSON object.").arg(filename);
    }

    {
#ifdef __APPLE__
        const auto default_font_family = QStringLiteral("SF Pro");
#else
        const auto default_font_family = QStringLiteral("Arial");
#endif

        m_qfont = QFont{};
        m_qfont.setFamily(
            get_json_string(obj, QStringLiteral("font_family")).value_or(default_font_family));

        if (m_qfont.family().isEmpty())
        {
            m_qfont.setFamily(default_font_family);
        }

        m_qfont.setBold(get_json_bool(obj, QStringLiteral("is_bold")).value_or(false));
        m_qfont.setItalic(get_json_bool(obj, QStringLiteral("is_italic")).value_or(false));
        m_qfont.setPixelSize(
            std::clamp(get_json_int(obj, QStringLiteral("font_pixel_size")).value_or(72), 8, 400));
    }

    m_max_page_extent =
        std::clamp(get_json_int(obj, QStringLiteral("max_page_extent")).value_or(0), 64, 4096 * 2);

    // TODO: padding is currently ignored
    m_padding = 0;

    m_anti_aliasing = get_json_bool(obj, u"anti_aliasing").value_or(true);

    const QString desc_str = get_json_string(obj, QStringLiteral("desc_type")).value_or(QString{});
    m_desc_type            = [desc_str] {
        if (desc_str == "json")
        {
            return FontDescriptionType::JSON;
        }
        if (desc_str == "xml")
        {
            return FontDescriptionType::XML;
        }
        if (desc_str == "text")
        {
            return FontDescriptionType::Text;
        }

        return static_cast<FontDescriptionType>(-1);
    }();

    if (m_desc_type == FontDescriptionType(-1))
    {
        return tr("Invalid font type '%1'").arg(desc_str);
    }

    const QString image_type_str =
        get_json_string(obj, QStringLiteral("image_type")).value_or(QString{});

    m_image_type = font_export_image_type_from_string(image_type_str);
    if (m_image_type == FontExportImageType(-1))
    {
        return tr("Invalid image type '%s'").arg(image_type_str);
    }

    m_export_directory =
        get_json_string(obj, QStringLiteral("ExportDirectory")).value_or(QString{});

    if (m_export_directory.isEmpty())
    {
        m_export_directory = s_default_export_directory;
    }

    m_should_flip_images_upside_down =
        get_json_bool(obj, QStringLiteral("flip_images_upside_down")).value_or(false);

    m_preview_background_color = get_json_color(obj, QStringLiteral("preview_background_color"))
                                     .value_or(QColor{54, 54, 54});

    m_preview_text = get_json_string(obj, QStringLiteral("preview_text")).value_or("Hello World!");

    {
        const auto [error_message, base_fill] = get_json_fill(obj, QStringLiteral("base_fill"));
        if (!error_message.isEmpty())
        {
            return error_message;
        }
        m_base_fill = base_fill;
    }

    {
        const auto [error_message, stroke_fill] = get_json_fill(obj, QStringLiteral("stroke_fill"));
        if (!error_message.isEmpty())
        {
            return error_message;
        }
        m_stroke_fill = stroke_fill;
    }

    m_stroke_spread =
        std::clamp(get_json_int(obj, QStringLiteral("stroke_spread")).value_or(0), 0, 100);

    m_stroke_style = parse_stroke_style(
        get_json_string(obj, QStringLiteral("stroke_style")).value_or(QString{}));

    m_stroke_cap_style =
        [s = get_json_string(obj, QStringLiteral("stroke_cap_style")).value_or(QString{})] {
            if (s == "Flat")
                return Qt::FlatCap;
            else if (s == "Square")
                return Qt::SquareCap;
            else if (s == "Round")
                return Qt::RoundCap;
            else
                return Qt::FlatCap;
        }();

    m_stroke_join_style =
        [s = get_json_string(obj, QStringLiteral("stroke_join_style")).value_or(QString{})] {
            if (s == "Miter")
                return Qt::MiterJoin;
            else if (s == "Bevel")
                return Qt::BevelJoin;
            else if (s == "Round")
                return Qt::RoundJoin;
            else
                return Qt::MiterJoin;
        }();

    m_stroke_miter_limit =
        std::clamp(get_json_int(obj, QStringLiteral("stroke_miter_limit")).value_or(200), 0, 200);

    m_stroke_dash_offset =
        std::clamp(get_json_int(obj, QStringLiteral("stroke_dash_offset")).value_or(0), 0, 100);

    m_characters = get_json_char_set(obj, QStringLiteral("chars"));
    m_variations = get_json_double_set(obj, QStringLiteral("variations"));

    if (m_variations.isEmpty())
    {
        m_variations.insert(1.0);
    }

    return QString{};
}

QString FontModel::directory() const
{
    return QFileInfo{m_filename}.absoluteDir().absolutePath();
}

QString FontModel::relative_filename(const QString& absolute_filename) const
{
    if (!QFileInfo{absolute_filename}.isAbsolute())
    {
        return absolute_filename;
    }
    else
    {
        auto dir = QDir{directory()};
        return dir.relativeFilePath(absolute_filename);
    }
}

QString FontModel::absolute_filename(const QString& relative_filename) const
{
    if (QFileInfo{relative_filename}.isAbsolute())
    {
        return relative_filename;
    }
    else
    {
        auto dir = QDir{directory()};
        return dir.absoluteFilePath(relative_filename);
    }
}

FontModel::operator bool() const
{
    return m_error_message.isEmpty();
}
