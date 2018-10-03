/**
 * \file filenameformatbox.cpp
 * Group box containing filename format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Nov 2017
 *
 * Copyright (C) 2017  Urs Fleisch
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

#include "filenameformatbox.h"
#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include "formatconfig.h"

/**
 * Constructor.
 *
 * @param title  title
 * @param parent parent widget
 */
FilenameFormatBox::FilenameFormatBox(const QString& title, QWidget* parent)
  : FormatBox(title, parent),
    m_maximumLengthCheckBox(nullptr), m_maximumLengthSpinBox(nullptr)
{
  if (auto formLayout = qobject_cast<QFormLayout*>(layout())) {
    m_maximumLengthCheckBox = new QCheckBox(tr("Maximum length:"));
    m_maximumLengthSpinBox = new QSpinBox;
    m_maximumLengthSpinBox->setMinimum(10);
    m_maximumLengthSpinBox->setMaximum(255);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->insertRow(1, m_maximumLengthCheckBox, m_maximumLengthSpinBox);
    connect(m_maximumLengthCheckBox, &QAbstractButton::toggled,
            m_maximumLengthSpinBox, &QWidget::setEnabled);
  }
}

/**
 * Set the values from a format configuration.
 *
 * @param cfg format configuration
 */
void FilenameFormatBox::fromFormatConfig(const FormatConfig& cfg)
{
  FormatBox::fromFormatConfig(cfg);
  if (m_maximumLengthCheckBox) {
    m_maximumLengthCheckBox->setChecked(cfg.enableMaximumLength());
  }
  if (m_maximumLengthSpinBox) {
    m_maximumLengthSpinBox->setValue(cfg.maximumLength());
    m_maximumLengthSpinBox->setEnabled(cfg.enableMaximumLength());
  }
}

/**
 * Store the values in a format configuration.
 *
 * @param cfg format configuration
 */
void FilenameFormatBox::toFormatConfig(FormatConfig& cfg) const
{
  FormatBox::toFormatConfig(cfg);
  if (m_maximumLengthCheckBox) {
    cfg.setEnableMaximumLength(m_maximumLengthCheckBox->isChecked());
  }
  if (m_maximumLengthSpinBox) {
    cfg.setMaximumLength(m_maximumLengthSpinBox->value());
  }
}
