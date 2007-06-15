/**
 * \file flacframelist.h
 * List of FLAC comment frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
 */

#ifndef FLACFRAMELIST_H
#define FLACFRAMELIST_H

#include "config.h"
#ifdef HAVE_FLAC

#include "oggframelist.h"

/**
 * List of Flac comment frames.
 */
class FlacFrameList : public OggFrameList {
public:
	/**
	 * Constructor.
	 */
	FlacFrameList();

	/**
	 * Destructor.
	 */
	virtual ~FlacFrameList();

	/**
	 * Set file and fill the list box with its frames.
	 * The listbox has to be set with setListBox() before calling this
	 * function.
	 *
	 * @param taggedFile file
	 */
	virtual void setTags(TaggedFile* taggedFile);

private:
	FlacFrameList(const FlacFrameList&);
	FlacFrameList& operator=(const FlacFrameList&);
};

#endif // HAVE_FLAC

#endif // FLACFRAMELIST_H
