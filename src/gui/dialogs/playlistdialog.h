/**
 * \file playlistdialog.h
 * Create playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2009
 *
 * Copyright (C) 2009-2024  Urs Fleisch
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

class QRadioButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class PlaylistConfig;

/**
 * Playlist dialog.
 */
class PlaylistDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent  parent widget
   */
  explicit PlaylistDialog(QWidget* parent);

  /**
   * Destructor.
   */
  ~PlaylistDialog() override = default;

  /**
   * Read the local settings from the configuration.
   */
  void readConfig();

  /**
   * Get the current dialog configuration.
   *
   * @param cfg the current configuration is returned here
   */
  void getCurrentConfig(PlaylistConfig& cfg) const;

  /**
   * Get the entered file name to create a new empty playlist.
   * @return file name if "Create new empty playlist" is selected, else empty.
   */
  QString getFileNameForNewEmptyPlaylist() const;

private slots:
  /**
   * Save the local settings to the configuration.
   */
  void saveConfig() const;

  /**
   * Show help.
   */
  void showHelp();

private:
  QRadioButton* m_sameAsDirNameButton;
  QRadioButton* m_fileNameFormatButton;
  QRadioButton* m_fileNameForEmptyButton;
  QLineEdit* m_fileNameForEmptyEdit;
  QComboBox* m_locationComboBox;
  QComboBox* m_formatComboBox;
  QCheckBox* m_onlySelectedFilesCheckBox;
  QRadioButton* m_sortFileNameButton;
  QRadioButton* m_sortTagFieldButton;
  QRadioButton* m_relPathButton;
  QRadioButton* m_fullPathButton;
  QRadioButton* m_writeListButton;
  QRadioButton* m_writeInfoButton;
  QComboBox* m_fileNameFormatComboBox;
  QComboBox* m_sortTagFieldComboBox;
  QComboBox* m_writeInfoComboBox;
};
