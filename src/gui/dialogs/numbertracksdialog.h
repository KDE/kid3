/**
 * \file numbertracksdialog.h
 * Number tracks dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 May 2006
 *
 * Copyright (C) 2006-2018  Urs Fleisch
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
#include "trackdata.h"

class QSpinBox;
class QComboBox;
class QCheckBox;

/**
 * Number tracks dialog.
 */
class NumberTracksDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  explicit NumberTracksDialog(QWidget* parent);

  /**
   * Destructor.
   */
  virtual ~NumberTracksDialog() override = default;

  /**
   * Get start number.
   */
  int getStartNumber() const;

  /**
   * Get destination.
   *
   * @return TagV1, TagV2 or TagV2V1 if ID3v1, ID2v2 or both are destination
   */
  Frame::TagVersion getDestination() const;

  /**
   * Set the total number of tracks.
   *
   * @param numTracks number of tracks
   * @param enable    true to enable setting of total
   */
  void setTotalNumberOfTracks(int numTracks, bool enable);

  /**
   * Get the total number of tracks.
   *
   * @param enable true is returned here if total number of tracks is checked
   *
   * @return number of tracks entered
   */
  int getTotalNumberOfTracks(bool* enable) const;

  /**
   * Check if track numbering is enabled.
   * @return true if enabled.
   */
  bool isTrackNumberingEnabled() const;

  /**
   * Check if counter has to be reset for each directory.
   * @return true if enabled.
   */
  bool isDirectoryCounterResetEnabled() const;

private slots:
  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

private:
  /** check box to enable track numbering */
  QCheckBox* m_numberTracksCheckBox;
  /** spinbox with starting track number */
  QSpinBox* m_trackSpinBox;
  /** combobox with destination */
  QComboBox* m_destComboBox;
  /** checkbox to reset counter for each directory*/
  QCheckBox* m_resetCounterCheckBox;
  /** total number of tracks checkbox */
  QCheckBox* m_totalNumTracksCheckBox;
  /** spinbox with total number of tracks */
  QSpinBox* m_totalNumTrackSpinBox;
};
