// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "Slider.hpp"

#include <QWheelEvent>

Slider::Slider(QWidget* parent)
    : QSlider(parent)
{
}

void Slider::wheelEvent(QWheelEvent* event)
{
    event->ignore();
}
