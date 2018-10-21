/**
 * \file tagformatbox.cpp
 * Group box containing tag format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Nov 2017
 *
 * Copyright (C) 2017-2018  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tagformatbox.h"
#include <QFormLayout>
#include <QCheckBox>
#include "formatconfig.h"

/**
 * Constructor.
 *
 * @param title  title
 * @param parent parent widget
 */
TagFormatBox::TagFormatBox(const QString& title, QWidget* parent)
  : FormatBox(title, parent),
    m_validationCheckBox(nullptr)
{
  if (auto formLayout = qobject_cast<QFormLayout*>(layout())) {
    m_validationCheckBox = new QCheckBox(tr("Validation"));
    formLayout->insertRow(1, m_validationCheckBox);
  }
}

/**
 * Set the values from a format configuration.
 *
 * @param cfg format configuration
 */
void TagFormatBox::fromFormatConfig(const FormatConfig& cfg)
{
  FormatBox::fromFormatConfig(cfg);
  if (m_validationCheckBox) {
    m_validationCheckBox->setChecked(cfg.enableValidation());
  }
}

/**
 * Store the values in a format configuration.
 *
 * @param cfg format configuration
 */
void TagFormatBox::toFormatConfig(FormatConfig& cfg) const
{
  FormatBox::toFormatConfig(cfg);
  if (m_validationCheckBox) {
    cfg.setEnableValidation(m_validationCheckBox->isChecked());
  }
}
