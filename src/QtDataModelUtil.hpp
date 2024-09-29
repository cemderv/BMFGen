// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#define DEFINE_PROPERTY(type, name, signal)                                                        \
  private:                                                                                         \
    type m_##name{};                                                                               \
                                                                                                   \
  public:                                                                                          \
    type name() const                                                                              \
    {                                                                                              \
        return m_##name;                                                                           \
    }                                                                                              \
    void set_##name(const type& value)                                                             \
    {                                                                                              \
        if (value != m_##name)                                                                     \
        {                                                                                          \
            m_##name = value;                                                                      \
            emit signal();                                                                         \
        }                                                                                          \
    }
