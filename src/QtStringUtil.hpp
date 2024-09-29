// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QString>

class QtStringUtil final
{
  public:
    QtStringUtil() = delete;

    static QString shorten_with_triple_dot(const QString& str, int max_length);
};
