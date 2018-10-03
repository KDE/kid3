/**
 * \file batchimportdialog.h
 * Batch import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#ifndef BATCHIMPORTDIALOG_H
#define BATCHIMPORTDIALOG_H

#include <QDialog>
#include "batchimportprofile.h"
#include "trackdata.h"

class QPushButton;
class QTextEdit;
class QComboBox;
class ServerImporter;
class BatchImportSourcesModel;

/**
 * Batch import dialog.
 */
class BatchImportDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param importers     server importers
   * @param parent parent widget
   */
  explicit BatchImportDialog(const QList<ServerImporter*>& importers,
                             QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~BatchImportDialog() override = default;

  /**
   * Read the local settings from the configuration.
   */
  void readConfig();

signals:
  /**
   * Start batch import with given @a profile.
   * @param profile batch import profile
   * @param tagVersion import destination tag versions
   */
  void start(const BatchImportProfile& profile,
             Frame::TagVersion tagVersion);

  /**
   * Abort batch import.
   */
  void abort();

public slots:
  /**
   * Show information about import event.
   * @param type import event type, enum BatchImporter::ImportEventType
   * @param text text to display
   */
  void showImportEvent(int type, const QString& text);

private slots:
  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

  /**
   * Start or abort batch import.
   */
  void startOrAbortImport();

  /**
   * Add a new profile.
   */
  void addProfile();

  /**
   * Remove the selected profile.
   */
  void removeProfile();

  /**
   * Switch to different profile.
   * @param index combo box index to set
   */
  void changeProfile(int index);

  /**
   * Change name of current profile.
   * @param name profile name
   */
  void changeProfileName(const QString& name);

private:
  /**
   * Set the profile from the configuration.
   */
  void setProfileFromConfig();

  /**
   * Update profile from GUI controls.
   */
  void setProfileFromGuiControls();

  /**
   * Update GUI controls from profiles.
   */
  void setGuiControlsFromProfile();

  /**
   * Add a new profile to the list of profiles.
   */
  void addNewProfile();

  /**
   * Set button to Start or Abort.
   * @param enableAbort true to set Abort button
   */
  void setAbortButton(bool enableAbort);

  /** Text editor */
  QTextEdit* m_edit;
  /** Combo box with import destinations */
  QComboBox* m_destComboBox;
  /** Combo box with profile name */
  QComboBox* m_profileComboBox;
  /** Start/Abort button */
  QPushButton* m_startAbortButton;
  /** Model containing currently selected import sources */
  BatchImportSourcesModel* m_profileModel;
  /** Importers for different servers */
  QList<ServerImporter*> m_importers;
  /** Batch import profiles */
  QList<BatchImportProfile> m_profiles;
  /** Index of currently selected profile */
  int m_profileIdx;
  /** Currently used batch import profile */
  BatchImportProfile m_currentProfile;
  /** true if m_startAbortButton is an Abort button */
  bool m_isAbortButton;
};

#endif
