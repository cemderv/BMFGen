// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "QtStringUtil.hpp"

QString QtStringUtil::shorten_with_triple_dot(const QString& str, int max_length)
{
    return str.length() > max_length ? str.mid(0, max_length) + QStringLiteral("...") : str;
}
