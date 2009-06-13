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

#include <qlabel.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QByteArray>
#endif
#include "taggedfile.h"

class FrameTable;
class QPushButton;

/**
 * List of frames.
 */
class FrameList : public QObject {
public:
	/**
	 * Constructor.
	 *
	 * @param ft frame table
	 */
	FrameList(FrameTable* ft);

	/**
	 * Destructor.
	 */
	~FrameList();

	/**
	 * Clear listbox and file reference.
	 */
	void clear();

	/**
	 * Set file and fill the list box with its frames.
	 * The listbox has to be set with setListBox() before calling this
	 * function.
	 *
	 * @param taggedFile file
	 */
	void setTags(TaggedFile* taggedFile);

	/**
	 * Create dialog to edit the selected frame and update the fields
	 * if Ok is returned.
	 *
	 * @return true if Ok selected in dialog.
	 */
	bool editFrame();

	/**
	 * Delete selected frame.
	 *
	 * @return false if frame not found.
	 */
	bool deleteFrame();

	/**
	 * Add a new frame.
	 *
	 * @param edit    true to edit frame after adding it
	 *
	 * @return true if frame added.
	 */
	bool addFrame(bool edit = false);

	/**
	 * Paste the selected frame from the copy buffer.
	 *
	 * @return true if frame pasted.
	 */
	bool pasteFrame();

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
	 * Get file containing frames.
	 *
	 * @return file, NULL if no file selected.
	 */
	TaggedFile* getFile() const;

	/**
	 * Reload the frame list, keeping the same row selected.
	 */
	void reloadTags();

	/**
	 * Display a dialog to select a frame type.
	 *
	 * @return false if no frame selected.
	 */
	bool selectFrame();

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
	 * Get frame of selected frame list item.
	 *
	 * @param frame the selected frame is returned here
	 *
	 * @return false if not item is selected.
	 */
	bool getSelectedFrame(Frame& frame) const;

	/**
	 * Select the frame by ID.
	 *
	 * @param id ID of frame to select
	 */
	void setSelectedId(int id);

	/**
	 * Clear list box.
	 */
	void clearListBox();

private:
	/**
	 * Fill listbox with frame descriptions.
	 * Before using this method, the listbox and file have to be set.
	 * @see setListBox(), setTags()
	 */
	void readTags();

	/**
	 * Create dialog to edit a frame and update the fields
	 * if Ok is returned.
	 *
	 * @param frame frame to edit
	 *
	 * @return true if Ok selected in dialog.
	 */
	bool editFrame(Frame& frame);

	/** File containing tags */
	TaggedFile* m_file;
	/** Frame used to add, edit and paste */
	Frame m_frame;

	FrameTable* m_frameTable;

private:
	FrameList(const FrameList&);
	FrameList& operator=(const FrameList&);
};

#endif // FRAMELIST_H
