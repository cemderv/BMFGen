// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "CharacterSet.hpp"
#include <QListWidget>
#include <QSet>

class FontModel;

class CharacterSetSelectionWidget : public QListWidget
{
    Q_OBJECT

  public:
    explicit CharacterSetSelectionWidget(QWidget* parent = nullptr);

    void set_char_set(const CharacterSet& set);

    void recompute_items();

    void paintEvent(QPaintEvent* event) override;

  private:
    CharacterSet m_char_set;
    bool         m_ignore_char_selection_changed = false;
};
