/**
 * \file formatlistedit.h
 * Widget to edit a format list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Aug 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef FORMATLISTEDIT_H
#define FORMATLISTEDIT_H

#include <QWidget>
#include <QList>
#include <QStringList>

class QComboBox;
class QLineEdit;
class QPushButton;

/**
 * Widget to edit a format list.
 */
class FormatListEdit : public QWidget {
Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param labels list of label texts for fields in a single format
   * @param tooltips list of tooltips, one string per label, empty if not used
   * @param parent parent widget
   */
  FormatListEdit(const QStringList& labels,
                 const QStringList& tooltips,
                 QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~FormatListEdit() override;

  /**
   * Set format strings.
   *
   * @param formats list of format stringlists, the first stringlist contains
   *   the names, the second the corresponding string for the first line edit,
   *   etc.
   * @param index index to select, -1 to keep current index
   */
  void setFormats(const QList<QStringList>& formats, int index = -1);

  /**
   * Get format strings.
   *
   * @param index  if not null, the current index is returned here
   *
   * @return list of format stringlists, the first stringlist contains
   *   the names, the second the corresponding string for the first line edit,
   *   etc.
   */
  QList<QStringList> getFormats(int* index = nullptr);

  /**
   * Get a format string from the format currently displayed in the GUI.
   *
   * @param formatNr index of the format stringlist, 0 is the format name
   *   1 the first line edit, etc.
   *
   * @return format string.
   */
  QString getCurrentFormat(int formatNr) const;

signals:
  /**
   * Emitted when another format is selected or return is pressed in a lineedit.
   */
  void formatChanged();

private slots:
  /**
   * Set the currently selected format from the contents of the controls.
   */
  void commitCurrentEdits();

  /**
   * Set the format lineedits to the format of the index.
   *
   * @param index selected item in combo box
   */
  void updateLineEdits(int index);

  /**
   * Add a new item.
   */
  void addItem();

  /**
   * Remove the selected item.
   */
  void removeItem();

private:
  /**
   * Update GUI controls from formats.
   *
   * @param index combo box index to set
   */
  void updateComboBoxAndLineEdits(int index);

  QList<QStringList> m_formats;
  QComboBox* m_formatComboBox;
  QList<QLineEdit*> m_lineEdits;
  QPushButton* m_addPushButton;
  QPushButton* m_removePushButton;
};

#endif // FORMATLISTEDIT_H
