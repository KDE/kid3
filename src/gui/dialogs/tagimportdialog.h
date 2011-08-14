/**
 * \file tagimportdialog.h
 * Dialog to import from other tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Jun 2011
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

#ifndef TAGIMPORTDIALOG_H
#define TAGIMPORTDIALOG_H

#include <QDialog>

class FormatListEdit;
class TrackDataModel;

/**
 * Dialog to import from a text (file or clipboard).
 */
class TagImportDialog : public QDialog {
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param parent  parent widget
   * @param trackDataModel track data to be filled with imported values
   */
  explicit TagImportDialog(QWidget* parent,
                           TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  virtual ~TagImportDialog();

  /**
   * Clear dialog data.
   */
  void clear();

private slots:
  /**
   * Apply import to track data.
   */
  void apply();

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

signals:
  /**
   * Emitted when the m_trackDataVector was updated with new imported data.
   */
  void trackDataUpdated();

private:
  /**
   * Set the format combo box and line edits from the configuration.
   */
  void setFormatFromConfig();

  FormatListEdit* m_formatListEdit;
  TrackDataModel* m_trackDataModel;
};

#endif
