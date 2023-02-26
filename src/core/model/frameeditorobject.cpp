/**
 * \file frameeditorobject.cpp
 * IFrameEditor interface to QObject bridge.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Sep 2014
 *
 * Copyright (C) 2014-2023  Urs Fleisch
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

#include "frameeditorobject.h"
#include "frameobjectmodel.h"
#include "taggedfile.h"

/**
 * Constructor.
 * @param parent parent object
 */
FrameEditorObject::FrameEditorObject(QObject* parent) : QObject(parent),
  m_selectFrame(nullptr), m_editFrameTaggedFile(nullptr), m_frameObjectModel(nullptr),
  m_tagNr(Frame::Tag_2)
{
}

/**
 * Let user edit a frame and then update the fields
 * when the edits are accepted.
 * frameEdited() is emitted when the edit dialog is closed with the edited
 * frame as a parameter if it was accepted.
 *
 * @param frame frame to edit
 * @param taggedFile tagged file where frame has to be set
 */
void FrameEditorObject::editFrameOfTaggedFile(const Frame* frame,
                                               TaggedFile* taggedFile)
{
  if (!frame || !taggedFile) {
    emit frameEdited(m_tagNr, nullptr);
    return;
  }

  m_editFrame = *frame;
  m_editFrameTaggedFile = taggedFile;
  if (!m_frameObjectModel) {
    m_frameObjectModel = new FrameObjectModel(this);
  }
  m_frameObjectModel->setFrame(m_editFrame);
  emit frameEditRequested(m_frameObjectModel);
}

/**
 * Called when the frame edit dialog is closed.
 *
 * @param frame frame object model, null if canceled
 *
 * @see frameEditRequested()
 */
void FrameEditorObject::onFrameEditFinished(FrameObjectModel* frame)
{
  if (frame) {
    m_editFrame = frame->getFrame();
    if (m_editFrameTaggedFile->setFrame(m_tagNr, m_editFrame)) {
      m_editFrameTaggedFile->markTagChanged(m_tagNr,
                                            m_editFrame.getExtendedType());
    }
    emit frameEdited(m_tagNr, &m_editFrame);
  } else {
    emit frameEdited(m_tagNr, nullptr);
  }
}

/**
 * Let user select a frame type.
 * frameSelected() is emitted when the edit dialog is closed with the selected
 * frame as a parameter if a frame is selected.
 *
 * @param frame is filled with the selected frame
 * @param taggedFile tagged file for which frame has to be selected
 */
void FrameEditorObject::selectFrame(Frame* frame, const TaggedFile* taggedFile)
{
  if (taggedFile && frame) {
    QStringList frameNames = taggedFile->getFrameIds(m_tagNr);
    m_displayNameMap = Frame::getDisplayNameMap(frameNames);
    m_selectFrame = frame;
    emit frameSelectionRequested(m_displayNameMap.keys());
  }
}

/**
 * Called when the frame selection dialog is closed.
 *
 * @param displayName name of selected frame, empty if canceled
 */
void FrameEditorObject::onFrameSelectionFinished(const QString& displayName)
{
  if (!displayName.isEmpty()) {
    QString name = m_displayNameMap.value(displayName, displayName);
    m_displayNameMap.clear();
    Frame::Type type = Frame::getTypeFromName(name);
    *m_selectFrame = Frame(type, QLatin1String(""), name, -1);
    emit frameSelected(m_tagNr, m_selectFrame);
  } else {
    emit frameSelected(m_tagNr, nullptr);
  }
}

/**
 * Return object which emits frameSelected(), frameEdited() signals.
 *
 * @return object which emits signals.
 */
QObject* FrameEditorObject::qobject()
{
  return this;
}
