/**
 * \file chaptereditor.cpp
 * Editor for chapter frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Sep 2015
 *
 * Copyright (C) 2015-2018  Urs Fleisch
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

#include "chaptereditor.h"
#include <QFormLayout>
#include <QLineEdit>
#include "timeeventmodel.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
ChapterEditor::ChapterEditor(QWidget* parent)
  : QWidget(parent)
{
  setObjectName(QLatin1String("ChapterEditor"));
  auto formatLayout = new QFormLayout(this);
  formatLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  QString inputMask(QLatin1String("HHHHHHHH"));
  m_startTimeEdit = new QLineEdit;
  m_endTimeEdit = new QLineEdit;
  m_startOffsetEdit = new QLineEdit;
  m_startOffsetEdit->setInputMask(inputMask);
  m_endOffsetEdit = new QLineEdit;
  m_endOffsetEdit->setInputMask(inputMask);
  formatLayout->addRow(tr("Start time"), m_startTimeEdit);
  formatLayout->addRow(tr("End time"), m_endTimeEdit);
  formatLayout->addRow(tr("Start offset"), m_startOffsetEdit);
  formatLayout->addRow(tr("End offset"), m_endOffsetEdit);
}

/**
 * Set start and end time of chapter.
 * @param startTimeMs start time in ms
 * @param endTimeMs end time in ms
 * @param startOffset offset of first byte of chapter in file,
 *                    ignored if all ones
 * @param endOffset offset of byte after last chapter byte,
 *                  ignored if all ones
 */
void ChapterEditor::setValues(quint32 startTimeMs, quint32 endTimeMs,
                              quint32 startOffset, quint32 endOffset)
{
  m_startTimeEdit->setText(TimeEventModel::timeStampToString(startTimeMs));
  m_endTimeEdit->setText(TimeEventModel::timeStampToString(endTimeMs));
  m_startOffsetEdit->setText(QString::number(startOffset, 16).toUpper());
  m_endOffsetEdit->setText(QString::number(endOffset, 16).toUpper());
}

/**
 * Get start and end time of chapter.
 * @param startTimeMs the start time in ms is returned here
 * @param endTimeMs the end time in ms is returned here
 * @param startOffset the offset of the first byte of chapter in file is
 *                    returned here, ignored if all ones
 * @param endOffset the offset of the byte after the last chapter byte is
 *                  returned here, ignored if all ones
 */
void ChapterEditor::getValues(quint32& startTimeMs, quint32& endTimeMs,
                              quint32& startOffset, quint32& endOffset) const
{
  startTimeMs = TimeEventModel::timeStampFromString(m_startTimeEdit->text());
  endTimeMs = TimeEventModel::timeStampFromString(m_endTimeEdit->text());
  bool ok;
  startOffset = m_startOffsetEdit->text().toUInt(&ok, 16);
  if (!ok) {
    startOffset = 0xffffffff;
  }
  endOffset = m_endOffsetEdit->text().toUInt(&ok, 16);
  if (!ok) {
    endOffset = 0xffffffff;
  }
}
