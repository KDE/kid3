/**
 * \file subframeseditor.h
 * Editor for subframes contained in a frame.
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

#ifndef SUBFRAMESEDITOR_H
#define SUBFRAMESEDITOR_H

#include <QWidget>
#include "frame.h"

class QPushButton;
class FrameTableModel;
class FrameTable;
class EditFrameFieldsDialog;
class IPlatformTools;
class Kid3Application;
class TaggedFile;

/**
 * Editor for subframes contained in a frame.
 */
class SubframesEditor : public QWidget {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform tools
   * @param app application context
   * @param taggedFile tagged file
   * @param parent parent widget
   */
  explicit SubframesEditor(IPlatformTools* platformTools, Kid3Application* app,
                           const TaggedFile* taggedFile, QWidget* parent = 0);

  /**
   * Destructor.
   */
  virtual ~SubframesEditor();

  /**
   * Set subframes.
   * @param frames subframes, will be cleared
   */
  void setFrames(FrameCollection& frames);

  /**
   * Get subframes.
   * @param frames the subframes are returned here
   */
  void getFrames(FrameCollection& frames) const;

private slots:
  void onEditFrameDialogFinished(int result);
  void onEditClicked();
  void onAddClicked();
  void onDeleteClicked();

private:
  void editFrame(const Frame& frame, int row);

  IPlatformTools* m_platformTools;
  Kid3Application* m_app;
  const TaggedFile* m_taggedFile;
  FrameTableModel* m_frameTableModel;
  FrameTable* m_frameTable;
  QPushButton* m_editButton;
  QPushButton* m_addButton;
  QPushButton* m_deleteButton;
  EditFrameFieldsDialog* m_editFrameDialog;
  Frame m_editFrame;
  int m_editFrameRow;
};

#endif // SUBFRAMESEDITOR_H
