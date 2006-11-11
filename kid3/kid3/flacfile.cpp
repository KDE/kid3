/**
 * \file flacfile.cpp
 * Handling of FLAC files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
 */

#include "flacfile.h"
#ifdef HAVE_FLAC

#include "standardtags.h"
#include "genres.h"
#include "flacframelist.h"
#include <FLAC++/metadata.h>
#include <qfile.h>
#include <qdir.h>
#include <sys/stat.h>
#ifdef WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <cstdio>
#include <cmath>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3CString>
#endif

/**
 * Constructor.
 *
 * @param dn directory name
 * @param fn filename
 */
FlacFile::FlacFile(const QString& dn, const QString& fn) :
	OggFile(dn, fn), m_chain(0)
{
}

/**
 * Destructor.
 */
FlacFile::~FlacFile()
{
	if (m_chain) {
		delete m_chain;
	}
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void FlacFile::readTags(bool force)
{
	if (force || !m_fileRead) {
		m_comments.clear();
		changedV2 = false;
		m_fileRead = true;
		Q3CString fnIn = QFile::encodeName(dirname + QDir::separator() + filename);
		m_fileInfo.read(0); // just to start invalid
		if (!m_chain) {
			m_chain = new FLAC::Metadata::Chain;
		}
		if (m_chain && m_chain->is_valid()) {
			if (m_chain->read(fnIn)) {
				FLAC::Metadata::Iterator* mdit = new FLAC::Metadata::Iterator;
				if (mdit) {
					mdit->init(*m_chain);
					while (mdit->is_valid()) {
						::FLAC__MetadataType mdt = mdit->get_block_type();
						if (mdt == FLAC__METADATA_TYPE_STREAMINFO) {
							FLAC::Metadata::Prototype* proto = mdit->get_block();
							if (proto) {
								FLAC::Metadata::StreamInfo* si =
									dynamic_cast<FLAC::Metadata::StreamInfo*>(proto);
								m_fileInfo.read(si);
								delete proto;
							}
						} else if (mdt == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
							FLAC::Metadata::Prototype* proto = mdit->get_block();
							if (proto) {
								FLAC::Metadata::VorbisComment* vc =
									dynamic_cast<FLAC::Metadata::VorbisComment*>(proto);
								if (vc && vc->is_valid()) {
									unsigned numComments = vc->get_num_comments();
									for (unsigned i = 0; i < numComments; ++i) {
										FLAC::Metadata::VorbisComment::Entry entry =
											vc->get_comment(i);
										if (entry.is_valid()) {
											QString name =
												QString::fromUtf8(entry.get_field_name(),
																					entry.get_field_name_length()).
												stripWhiteSpace().upper();
											QString value =
												QString::fromUtf8(entry.get_field_value(),
																					entry.get_field_value_length()).
												stripWhiteSpace();
											if (!value.isEmpty()) {
												m_comments.push_back(
													CommentField(name, value));
											}
										}
									}
								}
								delete proto;
							}
						}
						if (!mdit->next()) {
							break;
						}
					}
					delete mdit;
				}
			}
		}
	}

	if (force) {
		new_filename = filename;
	}
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force   true to force writing even if file was not changed.
 * @param renamed will be set to true if the file was renamed,
 *                i.e. the file name is no longer valid, else *renamed
 *                is left unchanged
 * @param preserve true to preserve file time stamps
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool FlacFile::writeTags(bool force, bool* renamed, bool preserve)
{
	if (isChanged() &&
		!QFileInfo(dirname + QDir::separator() + filename).isWritable()) {
		return false;
	}

	if (m_fileRead && (force || changedV2) && m_chain && m_chain->is_valid()) {
		bool commentsSet = false;
		m_chain->sort_padding();
		FLAC::Metadata::Iterator* mdit = new FLAC::Metadata::Iterator;
		if (mdit) {
			mdit->init(*m_chain);
			while (mdit->is_valid()) {
				::FLAC__MetadataType mdt = mdit->get_block_type();
				if (mdt == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
					if (commentsSet) {
						mdit->delete_block(true);
					} else {
						FLAC::Metadata::Prototype* proto = mdit->get_block();
						if (proto) {
							FLAC::Metadata::VorbisComment* vc =
								dynamic_cast<FLAC::Metadata::VorbisComment*>(proto);
							if (vc && vc->is_valid()) {
								setVorbisComment(vc);
								commentsSet = true;
							}
							delete proto;
						}
					}
				}
				if (!mdit->next()) {
					if (!commentsSet) {
						FLAC::Metadata::VorbisComment* vc =
							new FLAC::Metadata::VorbisComment;
						if (vc) {
							if (vc->is_valid()) {
								setVorbisComment(vc);
								if (mdit->insert_block_after(vc)) {
									commentsSet = true;
								}
							}
							if (!commentsSet) {
								delete vc;
							}
						}
					}
					break;
				}
			}
			delete mdit;
		}
		if (commentsSet &&
				m_chain->write(true, preserve)) {
			changedV2 = false;
		} else {
			return false;
		}
	}
	if (new_filename != filename) {
		if (!renameFile(filename, new_filename)) {
			return false;
		}
		*renamed = true;
	}
	return true;
}

/**
 * Set the vorbis comment block with the comments.
 *
 * @param vc vorbis comment block to set
 */
void FlacFile::setVorbisComment(FLAC::Metadata::VorbisComment* vc)
{
	// first all existing comments are deleted
#ifndef HAVE_NO_FLAC_STREAMMETADATA_OPERATOR
	// the C++ API is not complete
	const ::FLAC__StreamMetadata* fsmd = *vc;
	FLAC__metadata_object_vorbiscomment_resize_comments(
		const_cast<FLAC__StreamMetadata*>(fsmd), 0);
#else
  const unsigned numComments = vc->get_num_comments();
  for (unsigned i = 0; i < numComments; ++i) {
    vc->delete_comment(0);
  }
#endif
	// then our comments are appended
	CommentList::iterator it = m_comments.begin();
	while (it != m_comments.end()) {
		QString name((*it).getName());
		QString value((*it).getValue());
		if (!value.isEmpty()) {
			// The number of bytes - not characters - has to be passed to the
			// Entry constructor, thus qstrlen is used.
			Q3CString valueCStr = value.utf8();
			vc->insert_comment(vc->get_num_comments(),
				FLAC::Metadata::VorbisComment::Entry(
					name.latin1(), valueCStr, qstrlen(valueCStr)));
			++it;
		} else {
			it = m_comments.erase(it);
		}
	}
}

/**
 * Get technical detail information.
 *
 * @return string with detail information,
 *         "" if no information available.
 */
QString FlacFile::getDetailInfo() const
{
	QString str;
	if (m_fileRead && m_fileInfo.valid) {
		str = QString("FLAC %1 kbps %2 Hz %3 Channels ").
			arg(m_fileInfo.bitrate / 1000).
			arg(m_fileInfo.sampleRate).
			arg(m_fileInfo.channels);
		if (m_fileInfo.duration > 0) {
			str += formatTime(m_fileInfo.duration);
		}
	}
	return str;
}

/**
 * Get duration of file.
 *
 * @return duration in seconds,
 *         0 if unknown.
 */
unsigned FlacFile::getDuration() const
{
	if (m_fileRead && m_fileInfo.valid) {
		return m_fileInfo.duration;
	}
	return 0;
}

/** Frame list for Flac files. */
FlacFrameList* FlacFile::s_flacFrameList = 0;

/**
 * Get frame list for this type of tagged file.
 *
 * @return frame list.
 */
FrameList* FlacFile::getFrameList() const
{
	if (!s_flacFrameList) {
		s_flacFrameList = new FlacFrameList();
	}
	return s_flacFrameList;
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".flac".
 */
QString FlacFile::getFileExtension() const
{
	return ".flac";
}

/**
 * Clean up static resources.
 */
void FlacFile::staticCleanup()
{
	delete s_flacFrameList;
	s_flacFrameList = 0;
}


/**
 * Read information about a FLAC file.
 * @param fn file name
 * @return true if ok.
 */
bool FlacFile::FileInfo::read(FLAC::Metadata::StreamInfo* si)
{
	if (si && si->is_valid()) {
		valid = true;
		channels = si->get_channels();
		sampleRate = si->get_sample_rate();
		duration = si->get_total_samples() / sampleRate;
		bitrate = si->get_bits_per_sample() * sampleRate;
	} else {
		valid = false;
	}
	return valid;
}

#endif // HAVE_FLAC
