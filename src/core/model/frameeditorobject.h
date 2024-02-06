/**
 * \file frameeditorobject.h
 * IFrameEditor interface to QObject bridge.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Sep 2014
 *
 * Copyright (C) 2014-2024  Urs Fleisch
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

#include <QObject>
#include <QMap>
#include "iframeeditor.h"
#include "frame.h"

class FrameObjectModel;

/**
 * IFrameEditor interface to QObject bridge.
 *
 * A FrameEditorObject can be used to use a QObject (e.g. a QML component) as a
 * frame editor. An instance is registered with FrameList::setFrameEditor() and
 * will communicate with the editor component using its
 * frameSelectionRequested() and frameEditRequested() signals and
 * onFrameSelectionFinished() and onFrameEditFinished() slots.
 */
class KID3_CORE_EXPORT FrameEditorObject : public QObject, public IFrameEditor {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit FrameEditorObject(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~FrameEditorObject() override = default;

  // IFrameEditor implementation

  /**
   * Let user edit a frame and then update the fields
   * when the edits are accepted.
   * frameEdited() is emitted when the edit dialog is closed with the edited
   * frame as a parameter if it was accepted.
   *
   * @param frame frame to edit
   * @param taggedFile tagged file where frame has to be set
   */
  void editFrameOfTaggedFile(const Frame* frame,
                             TaggedFile* taggedFile) override;

  /**
   * Let user select a frame type.
   * frameSelected() is emitted when the edit dialog is closed with the selected
   * frame as a parameter if a frame is selected.
   *
   * @param frame is filled with the selected frame
   * @param taggedFile tagged file for which frame has to be selected
   */
  void selectFrame(Frame* frame, const TaggedFile* taggedFile) override;

  /**
   * Return object which emits frameSelected(), frameEdited() signals.
   *
   * @return object which emits signals.
   */
  QObject* qobject() override;

  /**
   * Get the tag number of the edited frame.
   * @return tag number, default is Frame::Tag_2.
   */
  Frame::TagNumber tagNumber() const override { return m_tagNr; }

  /**
   * Set the tag number of the edited frame.
   * @param tagNr tag number
   */
  void setTagNumber(Frame::TagNumber tagNr) override { m_tagNr = tagNr; }

  // End of IFrameEditor implementation

public slots:
  /**
   * Called when the frame selection dialog is closed.
   *
   * @param displayName name of selected frame, empty if canceled
   *
   * @see frameSelectionRequested()
   */
  void onFrameSelectionFinished(const QString& displayName);

  /**
   * Called when the frame edit dialog is closed.
   *
   * @param frame frame object model, null if canceled
   *
   * @see frameEditRequested()
   */
  void onFrameEditFinished(const FrameObjectModel* frame);

signals:
  // IFrameEditor implementation

  /**
   * Emitted when the dialog to add and edit a frame is closed.
   * @param tagNr tag number
   * @param frame edited frame if dialog was accepted, else 0
   */
  void frameEdited(Frame::TagNumber tagNr, const Frame* frame);

  /**
   * Emitted when the dialog to select a frame is closed.
   * @param tagNr tag number
   * @param frame selected frame if dialog was accepted, else 0
   */
  void frameSelected(Frame::TagNumber tagNr, const Frame* frame);

  // End of IFrameEditor implementation

  /**
   * Emitted to request a frame selection from the frame editor.
   * When the frame selection is accepted or canceled,
   * onFrameSelectionFinished() shall be called.
   *
   * @param frameNames list of possible frame names
   */
  void frameSelectionRequested(const QStringList& frameNames);

  /**
   * Emitted to request a frame edit from the frame editor.
   * When the frame editing is finished, onFrameEditFinished() shall be called.
   *
   * @param frame frame object model
   */
  void frameEditRequested(FrameObjectModel* frame);

private:
  Frame* m_selectFrame;
  TaggedFile* m_editFrameTaggedFile;
  FrameObjectModel* m_frameObjectModel;
  Frame m_editFrame;
  QMap<QString, QString> m_displayNameMap;
  Frame::TagNumber m_tagNr;
};
