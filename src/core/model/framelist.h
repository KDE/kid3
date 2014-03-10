/**
 * \file framelist.h
 * List of frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2009  Urs Fleisch
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

#ifndef FRAMELIST_H
#define FRAMELIST_H

#include <QObject>
#include "frame.h"

class QItemSelectionModel;
class FrameTableModel;
class TaggedFile;
class IFrameEditor;

/**
 * List of frames.
 */
class FrameList : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param ftm frame table model
   * @param selModel item selection model
   */
  FrameList(FrameTableModel* ftm, QItemSelectionModel* selModel);

  /**
   * Destructor.
   */
  virtual ~FrameList();

  /**
   * Set tagged file.
   *
   * @param taggedFile file
   */
  void setTaggedFile(TaggedFile* taggedFile) { m_taggedFile = taggedFile; }

  /**
   * Get tagged file.
   * @return tagged file.
   */
  TaggedFile* getTaggedFile() const { return m_taggedFile; }

  /**
   * Delete selected frame.
   *
   * @return false if frame not found.
   */
  bool deleteFrame();

  /**
   * Let the user select and edit a frame type and then edit the frame.
   * Add the frame if the edits are accepted.
   * frameEdited() is emitted with the added frame.
   *
   * @param frameEditor frame editor
   */
  void selectAddAndEditFrame(IFrameEditor* frameEditor);

  /**
   * Add and edit a new frame.
   * frameEdited() is emitted with the added frame.
   *
   * @param frameEditor editor for frame fields
   */
  void addAndEditFrame(IFrameEditor* frameEditor);

  /**
   * Paste the selected frame from the copy buffer.
   *
   * @return true if frame pasted.
   */
  bool pasteFrame();

  /**
   * Get the frame in the copy buffer.
   * @return frame from copy buffer.
   */
  const Frame& getFrame() const { return m_frame; }

  /**
   * Set the frame in the copy buffer.
   * @param frame frame to set
   */
  void setFrame(const Frame& frame) { m_frame = frame; }

  /**
   * Check if the frame in the copy buffer is a picture frame.
   * @return true if picture frame.
   */
  bool isPictureFrame() const { return m_frame.getType() == Frame::FT_Picture; }

  /**
   * Get the name of the selected frame.
   *
   * @return name, QString::null if nothing selected.
   */
  QString getSelectedName() const;

  /**
   * Select a frame with a given name.
   *
   * @param name name of frame
   *
   * @return true if a frame with that name could be selected.
   */
  bool selectByName(const QString& name);

  /**
   * Get ID of selected frame list item.
   *
   * @return ID of selected item,
   *         -1 if not item is selected.
   */
  int getSelectedId() const;

  /**
   * Select the frame by ID.
   *
   * @param id ID of frame to select
   */
  void setSelectedId(int id);

signals:
  /**
   * Emitted when the dialog to add and edit a frame is closed.
   * @param frame edited frame if dialog was accepted, else 0
   */
  void frameEdited(const Frame* frame);

private slots:
  void onFrameEdited(const Frame* frame);

private:
  FrameList(const FrameList&);
  FrameList& operator=(const FrameList&);

  /**
   * Save the current cursor position.
   */
  void saveCursor();

  /**
   * Restore the cursor position saved with saveCursor().
   */
  void restoreCursor();

  /**
   * Get frame of selected frame list item.
   *
   * @param frame the selected frame is returned here
   *
   * @return false if not item is selected.
   */
  bool getSelectedFrame(Frame& frame) const;

  /**
   * Set the frame table model from the tagged file.
   */
  void setModelFromTaggedFile();

  /** Set of old changed frames stored while in the edit dialog */
  quint64 m_oldChangedFrames;
  /** File containing tags */
  TaggedFile* m_taggedFile;
  /** Frame used to add, edit and paste */
  Frame m_frame;

  FrameTableModel* m_frameTableModel;
  QItemSelectionModel* m_selectionModel;

  int m_cursorRow;
  int m_cursorColumn;
};

#endif // FRAMELIST_H
