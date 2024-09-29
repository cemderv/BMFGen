// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QGradientStops>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

static std::optional<QString> get_json_string(const QJsonObject& obj, QStringView name)
{
    const QJsonValue& val = obj[name];

    if (!val.isString())
    {
        qDebug("Expected JSON string for property '%s'", qPrintable(name.toString()));
        return std::nullopt;
    }

    return val.toString();
}

static std::optional<int> get_json_int(const QJsonObject& obj, QStringView name)
{
    const QJsonValue& val = obj[name];

    if (!val.isDouble())
    {
        qDebug("Expected JSON number for property '%s'", qPrintable(name.toString()));
        return std::nullopt;
    }

    return val.toInt();
}

static std::optional<double> get_json_double(const QJsonObject& obj, QStringView name)
{
    const QJsonValue& val = obj[name];

    if (!val.isDouble())
    {
        qDebug("Expected JSON number for property '%s'", qPrintable(name.toString()));
        return std::nullopt;
    }

    return val.toDouble();
}

static std::optional<bool> get_json_bool(const QJsonObject& obj, QStringView name)
{
    const QJsonValue& val = obj[name];

    if (!val.isBool())
    {
        qDebug("Expected JSON bool for property '%s'", qPrintable(name.toString()));
        return std::nullopt;
    }

    return val.toBool();
}

static std::optional<QColor> get_json_color(const QJsonObject& obj, QStringView name)
{
    const auto str = get_json_string(obj, name);
    if (!str.has_value())
    {
        qDebug("Expected JSON color for property '%s'", qPrintable(name.toString()));
        return std::nullopt;
    }

    return QColor::fromString(*str);
}

static QJsonValue color_to_json(const QColor& color)
{
    return QJsonValue::fromVariant(color.name(QColor::HexArgb));
}

static std::optional<QGradientStops> get_json_gradient_stops(const QJsonObject& obj,
                                                             QStringView        name)
{
    const QJsonValue& val = obj[name];

    if (!val.isArray())
    {
        return std::nullopt;
    }

    const QJsonArray arr = val.toArray();

    QGradientStops stops;

    for (const QJsonValueConstRef& elem_value : arr)
    {
        if (!elem_value.isObject())
        {
            return std::nullopt;
        }

        const QJsonObject elem_obj = elem_value.toObject();

        const std::optional<double> position = get_json_double(elem_obj, u"position");
        const std::optional<QColor> color    = get_json_color(elem_obj, u"color");

        if (!position.has_value() || !color.has_value())
        {
            return std::nullopt;
        }

        stops.append(QGradientStop{*position, *color});
    }

    return stops;
}

static inline QJsonArray gradient_stops_to_json(const QGradientStops& stops)
{
    QJsonArray arr;

    for (const std::pair<double, QColor>& stop : stops)
    {
        QJsonObject obj;
        obj.insert(u"position", stop.first);
        obj.insert(u"color", color_to_json(stop.second));

        arr.append(obj);
    }

    return arr;
}

static inline QJsonArray string_set_to_json(const QSet<QString>& set)
{
    QJsonArray arr;

    for (const auto& elem : set)
    {
        arr.append(QJsonValue::fromVariant(elem));
    }

    return arr;
}

static inline QJsonArray char_set_to_json(const QSet<QChar>& set)
{
    QJsonArray arr;

    for (const auto& elem : set)
    {
        arr.append(QJsonValue::fromVariant(elem.unicode()));
    }

    return arr;
}

static inline QSet<uint32_t> get_json_u_int_set(const QJsonObject& obj, QStringView name)
{
    const QJsonValue& value = obj[name];

    if (!value.isArray())
    {
        return {};
    }

    const QJsonArray arr = value.toArray();

    QSet<uint32_t> set;

    for (const auto& elem : arr)
    {
        set.insert(static_cast<uint32_t>(elem.toInteger()));
    }

    return set;
}

static QSet<QChar> get_json_char_set(const QJsonObject& obj, QStringView name)
{
    const QJsonValue& value = obj[name];

    if (!value.isArray())
    {
        return {};
    }

    const QJsonArray arr = value.toArray();

    QSet<QChar> set;

    for (const auto& elem : arr)
    {
        set.insert(QChar(elem.toInt()));
    }

    return set;
}

static QSet<double> get_json_double_set(const QJsonObject& obj, QStringView name)
{
    const QJsonValue& value = obj[name];

    if (!value.isArray())
    {
        return {};
    }

    const QJsonArray arr = value.toArray();

    QSet<double> set;

    for (const auto& elem : arr)
    {
        set.insert(elem.toDouble());
    }

    return set;
}

static inline QJsonArray uint_set_to_json(const QSet<uint32_t>& set)
{
    QJsonArray arr;

    for (const auto& elem : set)
    {
        arr.append(QJsonValue::fromVariant(qlonglong(elem)));
    }

    return arr;
}

static inline QJsonArray double_set_to_json(const QSet<double>& set)
{
    QJsonArray arr;

    for (const auto& elem : set)
    {
        arr.append(QJsonValue::fromVariant(elem));
    }

    return arr;
}

static inline QSet<QString> get_json_string_set(const QJsonObject& obj, QStringView name)
{
    const QJsonValue& value = obj[name];

    if (!value.isArray())
    {
        return {};
    }

    const QJsonArray arr = value.toArray();

    QSet<QString> set;

    for (const auto& elem : arr)
    {
        set.insert(elem.toString());
    }

    return set;
}
