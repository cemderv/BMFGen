// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontGenWorkerThread.hpp"

#include "FontModel.hpp"
#include "FontGenContext.hpp"

FontGenWorkerThread::FontGenWorkerThread(const FontModel* model, FontGenContext* context)
    : QThread(nullptr)
    , m_model(model)
    , m_context(context)
{
}

void FontGenWorkerThread::run()
{
    m_generated_font = m_context->generate_font(*m_model, {});
}

std::shared_ptr<GeneratedFont> FontGenWorkerThread::generated_font() const
{
    return m_generated_font;
}
