/**
 * \file iframeeditor.h
 * Interface for editor of frame fields.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jul 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#include "kid3api.h"
#include "frame.h"

class QObject;
class TaggedFile;

/**
 * Interface for editor of frame fields.
 */
class KID3_CORE_EXPORT IFrameEditor {
public:
  /**
   * Destructor.
   */
  virtual ~IFrameEditor();

  /**
   * Let user edit a frame and then update the fields
   * when the edits are accepted.
   * frameEdited() is emitted when the edit dialog is closed with the edited
   * frame as a parameter if it was accepted.
   *
   * @param frame frame to edit
   * @param taggedFile tagged file where frame has to be set
   */
  virtual void editFrameOfTaggedFile(const Frame* frame,
                                     TaggedFile* taggedFile) = 0;

  /**
   * Let user select a frame type.
   * frameSelected() is emitted when the edit dialog is closed with the selected
   * frame as a parameter if a frame is selected.
   *
   * @param frame is filled with the selected frame
   * @param taggedFile tagged file for which frame has to be selected
   */
  virtual void selectFrame(Frame* frame, const TaggedFile* taggedFile) = 0;

  /**
   * Return object which emits frameSelected(), frameEdited() signals.
   *
   * @return object which emits signals.
   */
  virtual QObject* qobject() = 0;

  /**
   * Get the tag number of the edited frame.
   * @return tag number.
   */
  virtual Frame::TagNumber tagNumber() const = 0;

  /**
   * Set the tag number of the edited frame.
   * @param tagNr tag number
   */
  virtual void setTagNumber(Frame::TagNumber tagNr) = 0;
};
