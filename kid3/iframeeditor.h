/**
 * \file iframeeditor.h
 * Interface for editor of frame fields.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jul 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef IFRAMEEDITOR_H
#define IFRAMEEDITOR_H

class Frame;
class TaggedFile;

/**
 * Interface for editor of frame fields.
 */
class IFrameEditor {
public:
	/**
	 * Destructor.
	 */
	virtual ~IFrameEditor() = 0;

	/**
	 * Let user edit a frame and then update the fields
	 * when the edits are accepted.
	 *
	 * @param frame frame to edit
	 * @param taggedFile tagged file where frame has to be set
	 *
	 * @return true if frame edits are accepted.
	 */
	virtual bool editFrameOfTaggedFile(Frame* frame, TaggedFile* taggedFile) = 0;

	/**
	 * Let user select a frame type.
	 *
	 * @param frame is filled with the selected frame if true is returned
	 * @param taggedFile tagged file for which frame has to be selected
	 *
	 * @return false if no frame selected.
	 */
	virtual bool selectFrame(Frame* frame, const TaggedFile* taggedFile) = 0;
};

#endif // IFRAMEEDITOR_H
