/**
 * \file chaptereditor.h
 * Editor for chapter frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Sep 2015
 *
 * Copyright (C) 2015  Urs Fleisch
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

#pragma once

#include <QWidget>

class QTimeEdit;
class QLineEdit;

/**
 * Editor for chapter frames.
 */
class ChapterEditor : public QWidget {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  explicit ChapterEditor(QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~ChapterEditor() override = default;

  /**
   * Set start and end time of chapter.
   * @param startTimeMs start time in ms
   * @param endTimeMs end time in ms
   * @param startOffset offset of first byte of chapter in file,
   *                    ignored if all ones
   * @param endOffset offset of byte after last chapter byte,
   *                  ignored if all ones
   */
  void setValues(quint32 startTimeMs, quint32 endTimeMs,
                 quint32 startOffset, quint32 endOffset);

  /**
   * Get start and end time of chapter.
   * @param startTimeMs the start time in ms is returned here
   * @param endTimeMs the end time in ms is returned here
   * @param startOffset the offset of the first byte of chapter in file is
   *                    returned here, ignored if all ones
   * @param endOffset the offset of the byte after the last chapter byte is
   *                  returned here, ignored if all ones
   */
  void getValues(quint32& startTimeMs, quint32& endTimeMs,
                 quint32& startOffset, quint32& endOffset) const;

private:
  QTimeEdit* m_startTimeEdit;
  QTimeEdit* m_endTimeEdit;
  QLineEdit* m_startOffsetEdit;
  QLineEdit* m_endOffsetEdit;
};
