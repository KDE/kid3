/**
 * \file framelist.cpp
 * List of ID3v2.3 frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2023  Urs Fleisch
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

#include "framelist.h"
#include <QItemSelectionModel>
#include "taggedfile.h"
#include "frametablemodel.h"
#include "iframeeditor.h"
#include "pictureframe.h"

/**
 * Constructor.
 *
 * @param tagNr tag number
 * @param ftm frame table model
 * @param selModel item selection model
 */
FrameList::FrameList(Frame::TagNumber tagNr,
                     FrameTableModel* ftm, QItemSelectionModel* selModel)
  : QObject(ftm), m_taggedFile(nullptr),
    m_frameEditor(nullptr), m_frameTableModel(ftm), m_selectionModel(selModel),
    m_cursorRow(-1), m_cursorColumn(-1), m_tagNr(tagNr), m_addingFrame(false)
{
  setObjectName(QLatin1String("FrameList"));
}

/**
 * Get ID of selected frame list item.
 *
 * @return ID of selected item,
 *         -1 if not item is selected.
 */
int FrameList::getSelectedId() const
{
  const Frame* currentFrame =
    m_frameTableModel->getFrameOfIndex(m_selectionModel->currentIndex());
  return currentFrame ? currentFrame->getIndex() : -1;
}

/**
 * Get frame of selected frame list item.
 *
 * @param frame the selected frame is returned here
 *
 * @return false if not item is selected.
 */
bool FrameList::getSelectedFrame(Frame& frame) const
{
  const Frame* currentFrame =
    m_frameTableModel->getFrameOfIndex(m_selectionModel->currentIndex());
  if (currentFrame) {
    frame = *currentFrame;
    return true;
  }
  return false;
}

/**
 * Select the frame by ID.
 *
 * @param id ID of frame to select
 */
void FrameList::setSelectedId(int id)
{
  m_selectionModel->setCurrentIndex(
    m_frameTableModel->index(
      m_frameTableModel->getRowWithFrameIndex(id), 0),
        QItemSelectionModel::SelectCurrent);
}

/**
 * Get the name of the selected frame.
 *
 * @return name, QString::null if nothing selected.
 */
QString FrameList::getSelectedName() const
{
  const Frame* currentFrame =
    m_frameTableModel->getFrameOfIndex(m_selectionModel->currentIndex());
  return currentFrame ? currentFrame->getName() : QString();
}

/**
 * Select a frame with a given name.
 *
 * @param name name of frame
 *
 * @return true if a frame with that name could be selected.
 */
bool FrameList::selectByName(const QString& name)
{
  return selectByRow(m_frameTableModel->getRowWithFrameName(name));
}

/**
 * Select a frame by row number in the frame table.
 *
 * @param row row of frame
 *
 * @return true if a frame could be selected.
 */
bool FrameList::selectByRow(int row)
{
  if (row < 0 || row >= m_frameTableModel->rowCount())
    return false;

  m_selectionModel->setCurrentIndex(m_frameTableModel->index(row, 0),
                                    QItemSelectionModel::SelectCurrent);
  return true;
}

/**
 * Set the frame table model from the tagged file.
 */
void FrameList::setModelFromTaggedFile()
{
  if (m_taggedFile) {
    FrameCollection frames;
    m_taggedFile->getAllFrames(m_tagNr, frames);
    m_frameTableModel->transferFrames(frames);
  }
}

/**
 * Delete selected frame.
 *
 * @return false if frame not found.
 */
bool FrameList::deleteFrame()
{
  saveCursor();
  Frame frame;
  if (getSelectedFrame(frame) && m_taggedFile) {
    m_taggedFile->deleteFrame(m_tagNr, frame);
    setModelFromTaggedFile();
    restoreCursor();
    return true;
  }
  return false;
}

/**
 * Set editor for frames.
 *
 * @param frameEditor frame editor
 */
void FrameList::setFrameEditor(IFrameEditor* frameEditor)
{
  if (m_frameEditor != frameEditor) {
    if (m_frameEditor) {
      QObject* obj = m_frameEditor->qobject();
      disconnect(obj, SIGNAL(frameSelected(Frame::TagNumber,const Frame*)), // clazy:exclude=old-style-connect
                 this, SLOT(onFrameSelected(Frame::TagNumber,const Frame*)));
      disconnect(obj, SIGNAL(frameEdited(Frame::TagNumber,const Frame*)), // clazy:exclude=old-style-connect
                 this, SLOT(onFrameEdited(Frame::TagNumber,const Frame*)));
    }
    m_frameEditor = frameEditor;
    if (m_frameEditor) {
      QObject* obj = m_frameEditor->qobject();
      connect(obj, SIGNAL(frameSelected(Frame::TagNumber,const Frame*)), // clazy:exclude=old-style-connect
              this, SLOT(onFrameSelected(Frame::TagNumber,const Frame*)));
      connect(obj, SIGNAL(frameEdited(Frame::TagNumber,const Frame*)), // clazy:exclude=old-style-connect
              this, SLOT(onFrameEdited(Frame::TagNumber,const Frame*)));
    }
  }
}

/**
 * Let the user select and edit a frame type and then edit the frame.
 * Add the frame if the edits are accepted.
 * frameEdited() is emitted with the added frame.
 */
void FrameList::selectAddAndEditFrame()
{
  if (m_taggedFile && m_frameEditor) {
    m_addingFrame = true;
    m_frameEditor->setTagNumber(m_tagNr);
    m_frameEditor->selectFrame(&m_frame, m_taggedFile);
  } else {
    emit frameAdded(nullptr);
  }
}

/**
 * Called when the frame is selected.
 * @param tagNr tag number
 * @param frame selected frame, 0 if none selected.
 */
void FrameList::onFrameSelected(Frame::TagNumber tagNr, const Frame* frame)
{
  if (tagNr != m_tagNr)
    return;

  if (frame) {
    addAndEditFrame();
  } else {
    emit frameAdded(nullptr);
  }
}

/**
 * Add and edit a new frame.
 * frameEdited() is emitted with the added frame.
 */
void FrameList::addAndEditFrame()
{
  if (m_taggedFile) {
    m_oldChangedFrames = m_taggedFile->getChangedFrames(m_tagNr);
    if (!m_taggedFile->addFrame(m_tagNr, m_frame)) {
      emit frameAdded(nullptr);
    } else if (m_frameEditor) {
      m_addingFrame = true;
      m_frameEditor->setTagNumber(m_tagNr);
      m_frameEditor->editFrameOfTaggedFile(&m_frame, m_taggedFile);
    } else {
      m_addingFrame = true;
      onFrameEdited(m_tagNr, &m_frame);
    }
  } else {
    emit frameAdded(nullptr);
  }
}

/**
 * Edit the current frame.
 * The frame and its file have to be set using setFrame() and setTaggedFile().
 */
void FrameList::editFrame()
{
  if (m_frameEditor) {
    m_addingFrame = false;
    m_frameEditor->setTagNumber(m_tagNr);
    m_frameEditor->editFrameOfTaggedFile(&m_frame, m_taggedFile);
  }
}

/**
 * Called when the frame is edited.
 * @param tagNr tag number
 * @param frame edited frame, 0 if canceled
 */
void FrameList::onFrameEdited(Frame::TagNumber tagNr, const Frame* frame)
{
  if (tagNr != m_tagNr)
    return;

  if (frame) {
    int index = frame->getIndex();
    setModelFromTaggedFile();
    if (index != -1) {
      setSelectedId(index);
    }
  } else {
    if (m_addingFrame) {
      m_taggedFile->deleteFrame(m_tagNr, m_frame);
      m_taggedFile->setChangedFrames(m_tagNr, m_oldChangedFrames);
    }
  }
  if (m_addingFrame) {
    emit frameAdded(frame);
  } else {
    emit frameEdited(frame);
  }
}

/**
 * Paste the selected frame from the copy buffer.
 *
 * @return true if frame pasted.
 */
bool FrameList::pasteFrame() {
  if (m_taggedFile && m_frame.getType() != Frame::FT_UnknownFrame) {
    m_taggedFile->addFrame(m_tagNr, m_frame);
    m_taggedFile->setFrame(m_tagNr, m_frame);
    return true;
  }
  return false;
}

/**
 * Add a suitable field list for the frame in the copy buffer if missing.
 */
void FrameList::addFrameFieldList()
{
  if (m_taggedFile) {
    m_taggedFile->addFieldList(m_tagNr, m_frame);
    if (m_frame.getFieldList().isEmpty() &&
        m_frame.getType() == Frame::FT_Picture) {
      PictureFrame::setFields(m_frame);
    }
  }
}

/**
 * Save the current cursor position.
 */
void FrameList::saveCursor()
{
  m_cursorRow = m_selectionModel->currentIndex().row();
  m_cursorColumn = m_selectionModel->currentIndex().column();
}

/**
 * Restore the cursor position saved with saveCursor().
 */
void FrameList::restoreCursor()
{
  int lastRow = m_frameTableModel->rowCount() - 1;
  if (m_cursorRow >= 0 && m_cursorColumn >= 0 && lastRow >= 0) {
    if (m_cursorRow > lastRow) {
      m_cursorRow = lastRow;
    }
    m_selectionModel->setCurrentIndex(
      m_frameTableModel->index(m_cursorRow, m_cursorColumn),
      QItemSelectionModel::SelectCurrent);
  }
}
