/**
 * \file framelist.cpp
 * List of ID3v2.3 frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

/**
 * Constructor.
 *
 * @param ftm frame table model
 * @param selModel item selection model
 */
FrameList::FrameList(FrameTableModel* ftm, QItemSelectionModel* selModel) :
  QObject(ftm), m_oldChangedFrames(0), m_taggedFile(0), m_frameTableModel(ftm),
  m_selectionModel(selModel), m_cursorRow(-1), m_cursorColumn(-1)
{
  setObjectName(QLatin1String("FrameList"));
}

/**
 * Destructor.
 */
FrameList::~FrameList() {}

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
  int row = m_frameTableModel->getRowWithFrameName(name);
  if (row < 0)
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
    m_taggedFile->getAllFramesV2(frames);
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
    m_taggedFile->deleteFrameV2(frame);
    setModelFromTaggedFile();
    restoreCursor();
    return true;
  }
  return false;
}

/**
 * Let the user select and edit a frame type and then edit the frame.
 * Add the frame if the edits are accepted.
 * frameEdited() is emitted with the added frame.
 *
 * @param frameEditor frame editor
 */
void FrameList::selectAddAndEditFrame(IFrameEditor* frameEditor)
{
  if (m_taggedFile) {
    Frame frame;
    if (frameEditor->selectFrame(&frame, m_taggedFile)) {
      m_frame = frame;
      addAndEditFrame(frameEditor);
      return;
    }
  }
  emit frameEdited(0);
}

/**
 * Add and edit a new frame.
 * frameEdited() is emitted with the added frame.
 *
 * @param frameEditor editor for frame fields
 */
void FrameList::addAndEditFrame(IFrameEditor* frameEditor)
{
  if (m_taggedFile) {
    m_oldChangedFrames = m_taggedFile->getChangedFramesV2();
    if (!m_taggedFile->addFrameV2(m_frame)) {
      emit frameEdited(0);
    } else if (frameEditor) {
      connect(frameEditor->frameEditedEmitter(),
              SIGNAL(frameEdited(const Frame*)),
              this, SLOT(onFrameEdited(const Frame*)), Qt::UniqueConnection);
      frameEditor->editFrameOfTaggedFile(&m_frame, m_taggedFile);
    } else {
      onFrameEdited(&m_frame);
    }
  } else {
    emit frameEdited(0);
  }
}

/**
 * Called when the frame is edited.
 * @param frame edited frame, 0 if canceled
 */
void FrameList::onFrameEdited(const Frame* frame)
{
  if (QObject* emitter = sender()) {
    disconnect(emitter, SIGNAL(frameEdited(const Frame*)),
               this, SLOT(onFrameEdited(const Frame*)));
  }
  if (frame) {
    int index = frame->getIndex();
    setModelFromTaggedFile();
    if (index != -1) {
      setSelectedId(index);
    }
  } else {
    m_taggedFile->deleteFrameV2(m_frame);
    m_taggedFile->setChangedFramesV2(m_oldChangedFrames);
  }
  emit frameEdited(frame);
}

/**
 * Paste the selected frame from the copy buffer.
 *
 * @return true if frame pasted.
 */
bool FrameList::pasteFrame() {
  if (m_taggedFile && m_frame.getType() != Frame::FT_UnknownFrame) {
    m_taggedFile->addFrameV2(m_frame);
    m_taggedFile->setFrameV2(m_frame);
    return true;
  }
  return false;
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
