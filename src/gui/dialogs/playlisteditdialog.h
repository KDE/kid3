/**
 * \file playlisteditdialog.h
 * Edit playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 05 Aug 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

class QDialogButtonBox;
class QItemSelectionModel;
class PlaylistModel;

/**
 * Edit playlist dialog.
 */
class PlaylistEditDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param model playlist model
   * @param selModel selection model of associated file proxy model
   * @param parent parent widget
   */
  PlaylistEditDialog(PlaylistModel* model, QItemSelectionModel* selModel,
                     QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~PlaylistEditDialog() override;
  
  /**
   * Get playlist model.
   * @return playlist model.
   */
  PlaylistModel* playlistModel() const { return m_playlistModel; }

protected:
  /**
   * Ask user before closing with unsaved modifications.
   * @param event close event
   */
  virtual void closeEvent(QCloseEvent* event) override;

private slots:
  void setModified(bool modified);
  void showHelp();

private:
  void setWindowCaption();

  QDialogButtonBox* m_buttonBox;
  PlaylistModel* m_playlistModel;
};
