// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "CharacterSet.hpp"

QSet<QChar> CharacterSet::chars() const
{
    Q_ASSERT(range_end <= range_end);

    QSet<QChar> set;
    set.reserve(range_end - range_start);

    for (uint32_t i = range_start; i <= range_end; ++i)
    {
        set.insert(QChar(i));
    }

    return set;
}

qsizetype CharacterSet::count() const
{
    return range_end - range_start;
}

std::optional<CharacterSet> CharacterSet::find(const QList<CharacterSet>& set_list,
                                               const QString&             name)
{
    const auto it = std::ranges::find_if(set_list, [&name](const CharacterSet& set) {
        return set.name==name;
    });

    return it != set_list.end() ? std::optional{*it} : std::nullopt;
}
