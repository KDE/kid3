/**
 * \file tagimportdialog.h
 * Dialog to import from other tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Jun 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#include <QDialog>
#include "frame.h"

class QComboBox;
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
   * @param trackDataModel track data to be filled with imported values,
   *        nullptr if dialog is used independent from import dialog
   */
  explicit TagImportDialog(QWidget* parent,
                           TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  virtual ~TagImportDialog() override = default;

  /**
   * Clear dialog data.
   */
  void clear();

  /**
   * Get import destination.
   * Is only available if dialog is not opened from import dialog.
   * @return TagV1, TagV2 or TagV2V1 for ID3v1, ID3v2 or both.
   */
  Frame::TagVersion getDestination() const;

  /**
   * Get selected source format.
   * @return source format.
   */
  QString getSourceFormat() const;

  /**
   * Get selected extraction format.
   * @return extraction format.
   */
  QString getExtractionFormat() const;

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

  /**
   * Get help text for format codes supported in extraction field.
   * @return help text.
   */
  static QString getExtractionToolTip();

  FormatListEdit* m_formatListEdit;
  TrackDataModel* m_trackDataModel;
  QComboBox* m_destComboBox;
};
