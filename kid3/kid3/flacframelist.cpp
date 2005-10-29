/**
 * \file flacframelist.cpp
 * List of FLAC comment frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
 */

#include "flacframelist.h"
#ifdef HAVE_FLAC

#include "flacfile.h"

/**
 * Constructor.
 */
FlacFrameList::FlacFrameList()
{
}

/**
 * Destructor.
 */
FlacFrameList::~FlacFrameList()
{
}

/**
 * Set file and fill the list box with its frames.
 * The listbox has to be set with setListBox() before calling this
 * function.
 *
 * @param taggedFile file
 */
void FlacFrameList::setTags(TaggedFile* taggedFile)
{
	m_file = taggedFile;
	FlacFile* flacFile = dynamic_cast<FlacFile*>(m_file);
	if (flacFile && flacFile->isTagInformationRead()) {
		m_tags = &flacFile->m_comments;
		readTags();
	}
}

#endif // HAVE_FLAC
