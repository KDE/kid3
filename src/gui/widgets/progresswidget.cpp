/**
 * \file progresswidget.cpp
 * Widget showing progress, similar to QProgressDialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Jan 2017
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

#include "progresswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

/**
 * Constructor.
 * @param parent parent widget
 */
ProgressWidget::ProgressWidget(QWidget* parent) : QFrame(parent),
  m_percentage(0), m_wasCanceled(false)
{
  setFrameShape(QFrame::StyledPanel);
  setFrameShadow(QFrame::Sunken);
  auto layout = new QVBoxLayout(this);
  m_title = new QLabel;
  QFont titleFont = font();
  titleFont.setPointSize(titleFont.pointSize() + 3);
  titleFont.setBold(true);
  m_title->setFont(titleFont);

  layout->addWidget(m_title);
  m_label = new QLabel;
  layout->addWidget(m_label);
  m_progress = new QProgressBar;
  layout->addWidget(m_progress);
  auto buttonLayout = new QHBoxLayout;
  m_cancelButton = new QPushButton(tr("&Cancel"));
  connect(m_cancelButton, &QAbstractButton::clicked, this, &ProgressWidget::onCancelClicked);
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_cancelButton);
  layout->addLayout(buttonLayout);
  layout->addStretch();
}

/**
 * Destructor.
 */
ProgressWidget::~ProgressWidget()
{
}

/**
 * Set title.
 * @param text title
 */
void ProgressWidget::setWindowTitle(const QString& text)
{
  m_title->setText(text);
}

/**
 * Set text of label.
 * @param text label, default is empty
 */
void ProgressWidget::setLabelText(const QString& text)
{
  m_label->setText(text);
}

/**
 * Set text of cancel button.
 * @param text button text, default is "Cancel"
 */
void ProgressWidget::setCancelButtonText(const QString& text)
{
  m_cancelButton->setText(text);
}

/**
 * Set minimum value.
 * @param minimum minimum value, default is 0
 */
void ProgressWidget::setMinimum(int minimum)
{
  m_progress->setMinimum(minimum);
}

/**
 * Set maximum value.
 * @param maximum maximum value, default is 100
 */
void ProgressWidget::setMaximum(int maximum)
{
  m_progress->setMaximum(maximum);
}

/**
 * Set current amount of progress made.
 * @param value progress value
 */
void ProgressWidget::setValue(int value)
{
  m_progress->setValue(value);
}

/**
 * Set value and maximum, but only if it changes the current percentage.
 *
 * This will have better performance by avoiding too many UI updates.
 *
 * @param value progress value
 * @param maximum maximum value
 */
void ProgressWidget::setValueAndMaximum(int value, int maximum)
{
  int percentage = maximum > 0 ? value * 100 / maximum : 0;
  if (m_percentage != percentage) {
    m_percentage = percentage;
    m_progress->setMaximum(maximum);
    m_progress->setValue(value);
  }
}

/**
 * Set format used for progress text.
 * @param format format string, can contain %p, %v, %m for
 *               percentage, value, total
 */
void ProgressWidget::setFormat(const QString& format)
{
  m_progress->setFormat(format);
}

/**
 * Reset the progress widget.
 */
void ProgressWidget::reset()
{
  m_progress->reset();
  m_percentage = 0;
  m_wasCanceled = false;
}

void ProgressWidget::onCancelClicked()
{
  m_wasCanceled = true;
  emit canceled();
}
