// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QSet>
#include <optional>

class CharacterSet
{
  public:
    QString  name;
    uint32_t range_start;
    uint32_t range_end;

    QSet<QChar> chars() const;

    qsizetype count() const;

    static std::optional<CharacterSet> find(const QList<CharacterSet>& set_list,
                                            const QString&             name);

    bool operator==(const CharacterSet&) const = default;

    bool operator!=(const CharacterSet&) const = default;
};
