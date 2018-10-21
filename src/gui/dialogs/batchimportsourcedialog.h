/**
 * \file batchimportdialog.h
 * Dialog to add or edit a batch import source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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
#include "batchimportprofile.h"

class QComboBox;
class QSpinBox;
class QCheckBox;

/**
 * Dialog to add or edit a batch import source.
 */
class BatchImportSourceDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit BatchImportSourceDialog(QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~BatchImportSourceDialog() override = default;

  /**
   * Fill batch import source from dialog controls.
   * @param source batch import source to be filled
   */
  void getSource(BatchImportProfile::Source& source);

  /**
   * Set dialog controls from batch import source.
   * @param source batch import source containing properties to be set
   */
  void setSource(const BatchImportProfile::Source& source);

  /**
   * Set names of import servers.
   * @param servers server names
   */
  void setServerNames(const QStringList& servers);

private:
  QComboBox* m_serverComboBox;
  QSpinBox* m_accuracySpinBox;
  QCheckBox* m_standardTagsCheckBox;
  QCheckBox* m_additionalTagsCheckBox;
  QCheckBox* m_coverArtCheckBox;
};
