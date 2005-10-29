/**
 * \file oggframelist.h
 * List of Ogg comment frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Sep 2005
 */

#ifndef OGGFRAMELIST_H
#define OGGFRAMELIST_H

#include "config.h"
#if defined HAVE_VORBIS || defined HAVE_FLAC

#include "framelist.h"
#include "oggfile.h"

/**
 * List of Ogg comment frames.
 */
class OggFrameList : public FrameList {
public:
	/**
	 * Constructor.
	 */
	OggFrameList();

	/**
	 * Destructor.
	 */
	virtual ~OggFrameList();

	/**
	 * Set file and fill the list box with its frames.
	 * The listbox has to be set with setListBox() before calling this
	 * function.
	 *
	 * @param taggedFile file
	 */
	virtual void setTags(TaggedFile* taggedFile);

	/**
	 * Create dialog to edit the selected frame and update the fields
	 * if Ok is returned.
	 *
	 * @return true if Ok selected in dialog.
	 */
	virtual bool editFrame();

	/**
	 * Delete selected frame.
	 *
	 * @return false if frame not found.
	 */
	virtual bool deleteFrame();

	/**
	 * Add a new frame.
	 *
	 * @param frameId ID of frame to add, from selectFrameId()
	 * @return true if frame added.
	 */
	virtual bool addFrame(int frameId);

	/**
	 * Display a dialog to select a frame type.
	 *
	 * @return ID of selected frame, to be passed to addFrame(),
	 *         -1 if no frame selected.
	 */
	virtual int selectFrameId();

protected:
	/**
	 * Fill listbox with frame descriptions.
	 * Before using this method, the listbox and file have to be set.
	 * @see setListBox(), setTags()
	 */
	void readTags();

	OggFile::CommentList* m_tags;
	QString m_selectedName;
};

#endif // HAVE_VORBIS

#endif // HAVE_VORBIS || defined HAVE_FLAC
