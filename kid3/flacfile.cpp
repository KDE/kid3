/**
 * \file flacfile.cpp
 * Handling of FLAC files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#include "flacfile.hpp"
#ifdef HAVE_FLAC

#include "genres.h"
#include "dirinfo.h"
#include "pictureframe.h"
#include <FLAC++/metadata.h>
#include <qfile.h>
#include <qdir.h>
#include <qimage.h>
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
#include <QByteArray>
#endif

/**
 * Constructor.
 *
 * @param di directory information
 * @param fn filename
 */
FlacFile::FlacFile(const DirInfo* di, const QString& fn) :
	OggFile(di, fn), m_chain(0)
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

#ifdef HAVE_FLAC_PICTURE
/**
 * Get the picture block as a picture frame.
 *
 * @param frame frame to set
 * @param pic   picture block to get
 */
static void getPicture(Frame& frame, const FLAC::Metadata::Picture* pic)
{
	QByteArray ba;
	QCM_duplicate(
		ba,
		reinterpret_cast<const char*>(pic->get_data()),
		pic->get_data_length());
	PictureFrame::setFields(
		frame,
		Frame::Field::TE_ISO8859_1,	"",
		QString::fromAscii(pic->get_mime_type()),
		static_cast<PictureFrame::PictureType>(pic->get_type()),
		QString::fromUtf8(
			reinterpret_cast<const char*>(pic->get_description())),
		ba);
	frame.setInternalName("Picture");
}

/**
 * Set the picture block with the picture frame.
 *
 * @param frame frame to get
 * @param pic picture block to set
 */
static void setPicture(const Frame& frame, FLAC::Metadata::Picture* pic)
{
	Frame::Field::TextEncoding enc;
	PictureFrame::PictureType pictureType = PictureFrame::PT_CoverFront;
	QString imgFormat, mimeType, description;
	QByteArray ba;
	PictureFrame::getFields(frame, enc, imgFormat, mimeType,
													pictureType, description, ba);
	QImage image;
	if (image.loadFromData(ba)) {
		pic->set_width(image.width());
		pic->set_height(image.height());
		pic->set_depth(image.depth());
		pic->set_colors(image.numColors());
	}
	pic->set_mime_type(mimeType.QCM_toAscii());
	pic->set_type(
		static_cast<FLAC__StreamMetadata_Picture_Type>(pictureType));
	pic->set_description(
		reinterpret_cast<const FLAC__byte*>(
			static_cast<const char*>(description.QCM_toUtf8())));
	pic->set_data(reinterpret_cast<const FLAC__byte*>(ba.data()), ba.size());
}
#endif // HAVE_FLAC_PICTURE

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void FlacFile::readTags(bool force)
{
	if (force || !m_fileRead) {
		m_comments.clear();
#ifdef HAVE_FLAC_PICTURE
		m_pictures.clear();
		int pictureNr = 0;
#endif
		markTag2Unchanged();
		m_fileRead = true;
		QCM_QCString fnIn = QFile::encodeName(getDirInfo()->getDirname() + QDir::separator() + currentFilename());
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
												QCM_trimmed().QCM_toUpper();
											QString value =
												QString::fromUtf8(entry.get_field_value(),
																					entry.get_field_value_length()).
												QCM_trimmed();
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
#ifdef HAVE_FLAC_PICTURE
						else if (mdt == FLAC__METADATA_TYPE_PICTURE) {
							FLAC::Metadata::Prototype* proto = mdit->get_block();
							if (proto) {
								FLAC::Metadata::Picture* pic =
									dynamic_cast<FLAC::Metadata::Picture*>(proto);
								if (pic) {
									Frame frame(Frame::FT_Picture, "", "", pictureNr++);
									getPicture(frame, pic);
									m_pictures.push_back(frame);
								}
								delete proto;
							}
						}
#endif
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
		setFilename(currentFilename());
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
		!QFileInfo(getDirInfo()->getDirname() + QDir::separator() + currentFilename()).isWritable()) {
		return false;
	}

	if (m_fileRead && (force || isTag2Changed()) && m_chain && m_chain->is_valid()) {
		bool commentsSet = false;
#ifdef HAVE_FLAC_PICTURE
		bool pictureSet = false;
		bool pictureRemoved = false;
		PictureList::iterator pictureIt = m_pictures.begin();
#endif
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
#ifdef HAVE_FLAC_PICTURE
				else if (mdt == FLAC__METADATA_TYPE_PICTURE) {
					if (pictureIt != m_pictures.end()) {
						FLAC::Metadata::Prototype* proto = mdit->get_block();
						if (proto) {
							FLAC::Metadata::Picture* pic =
								dynamic_cast<FLAC::Metadata::Picture*>(proto);
							if (pic) {
								setPicture(*pictureIt++, pic);
								pictureSet = true;
							}
							delete proto;
						}
					} else {
						mdit->delete_block(false);
						pictureRemoved = true;
					}
				} else if (mdt == FLAC__METADATA_TYPE_PADDING) {
					if (pictureIt != m_pictures.end()) {
						FLAC::Metadata::Picture* pic =
							new FLAC::Metadata::Picture;
						if (pic) {
							setPicture(*pictureIt, pic);
							if (mdit->set_block(pic)) {
								++pictureIt;
								pictureSet = true;
							} else {
								delete pic;
							}
						}
					} else if (pictureRemoved) {
						mdit->delete_block(false);
					}
				}
#endif
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
#ifdef HAVE_FLAC_PICTURE
					while (pictureIt != m_pictures.end()) {
						FLAC::Metadata::Picture* pic =
							new FLAC::Metadata::Picture;
						if (pic) {
							setPicture(*pictureIt, pic);
							if (mdit->insert_block_after(pic)) {
								pictureSet = true;
							} else {
								delete pic;
							}
						}
						++pictureIt;
					}
#endif
					break;
				}
			}
			delete mdit;
		}
#ifdef HAVE_FLAC_PICTURE
		if ((commentsSet || pictureSet) &&
				m_chain->write(!pictureRemoved, preserve)) {
			markTag2Unchanged();
		}
#else
		if (commentsSet &&
				m_chain->write(true, preserve)) {
			markTag2Unchanged();
		}
#endif
		else {
			return false;
		}
	}
	if (getFilename() != currentFilename()) {
		if (!renameFile(currentFilename(), getFilename())) {
			return false;
		}
		updateCurrentFilename();
		// link tags to new file name
		readTags(true);
		*renamed = true;
	}
	return true;
}

#ifdef HAVE_FLAC_PICTURE
/**
 * Check if file has an ID3v2 tag.
 *
 * @return true if a V2 tag is available.
 * @see isTagInformationRead()
 */
bool FlacFile::hasTagV2() const
{
	return OggFile::hasTagV2() || !m_pictures.empty();
}

/**
 * Set a frame in the tags 2.
 *
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool FlacFile::setFrameV2(const Frame& frame)
{
	if (frame.getType() == Frame::FT_Picture) {
		int index = frame.getIndex();
		if (index != -1 && index < static_cast<int>(m_pictures.size())) {
#if QT_VERSION >= 0x040000
			PictureList::iterator it = m_pictures.begin() + index;
#else
			PictureList::iterator it = m_pictures.at(index);
#endif
			if (it != m_pictures.end()) {
				*it = frame;
				PictureFrame::setDescription(*it, frame.getValue());
				markTag2Changed(Frame::FT_Picture);
				return true;
			}
		}
	}
	return OggFile::setFrameV2(frame);
}

/**
 * Add a frame in the tags 2.
 *
 * @param frame frame to add
 *
 * @return true if ok.
 */
bool FlacFile::addFrameV2(Frame& frame)
{
	if (frame.getType() == Frame::FT_Picture) {
		if (frame.getFieldList().empty()) {
			PictureFrame::setFields(
				frame, Frame::Field::TE_ISO8859_1, "JPG", "image/jpeg",
				PictureFrame::PT_CoverFront, "", QByteArray());
		}
		PictureFrame::setDescription(frame, frame.getValue());
		frame.setIndex(m_pictures.size());
		m_pictures.push_back(frame);
		markTag2Changed(Frame::FT_Picture);
		return true;
	}
	return OggFile::addFrameV2(frame);
}

/**
 * Delete a frame in the tags 2.
 *
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool FlacFile::deleteFrameV2(const Frame& frame)
{
	if (frame.getType() == Frame::FT_Picture) {
		int index = frame.getIndex();
		if (index != -1 && index < static_cast<int>(m_pictures.size())) {
#if QT_VERSION >= 0x040000
			m_pictures.removeAt(index);
#else
			PictureList::iterator it = m_pictures.at(index);
			m_pictures.erase(it);
#endif
			markTag2Changed(Frame::FT_Picture);
			return true;
		}
	}
	return OggFile::deleteFrameV2(frame);
}

/**
 * Remove ID3v2 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void FlacFile::deleteFramesV2(const FrameFilter& flt)
{
	if (flt.areAllEnabled() || flt.isEnabled(Frame::FT_Picture)) {
		m_pictures.clear();
		markTag2Changed(Frame::FT_Picture);
	}
	OggFile::deleteFramesV2(flt);
}

/**
 * Get all frames in tag 2.
 *
 * @param frames frame collection to set.
 */
void FlacFile::getAllFramesV2(FrameCollection& frames)
{
	OggFile::getAllFramesV2(frames);
	int i = 0;
	for (PictureList::iterator it = m_pictures.begin();
			 it != m_pictures.end();
			 ++it) {
		(*it).setIndex(i++);
		frames.insert(*it);
	}
}

/**
 * Get a list of frame IDs which can be added.
 *
 * @return list with frame IDs.
 */
QStringList FlacFile::getFrameIds() const
{
	QStringList lst(OggFile::getFrameIds());
#if QT_VERSION >= 0x040000
	QStringList::iterator it = lst.begin() + Frame::FT_Picture;
#else
	QStringList::iterator it = lst.at(Frame::FT_Picture);
#endif
	lst.insert(it, QCM_translate(Frame::getNameFromType(Frame::FT_Picture)));
	return lst;
}
#endif // HAVE_FLAC_PICTURE

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
		  QCM_QCString valueCStr = value.QCM_toUtf8();
			vc->insert_comment(vc->get_num_comments(),
				FLAC::Metadata::VorbisComment::Entry(
					name.QCM_latin1(), valueCStr, qstrlen(valueCStr)));
			++it;
		} else {
			it = m_comments.erase(it);
		}
	}
}

/**
 * Get technical detail information.
 *
 * @param info the detail information is returned here
 */
void FlacFile::getDetailInfo(DetailInfo& info) const
{
	if (m_fileRead && m_fileInfo.valid) {
		info.valid = true;
		info.format = "FLAC";
		info.bitrate = m_fileInfo.bitrate / 1000;
		info.sampleRate = m_fileInfo.sampleRate;
		info.channels = m_fileInfo.channels;
		info.duration = m_fileInfo.duration;
	} else {
		info.valid = false;
	}
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


/**
 * Create an FlacFile object if it supports the filename's extension.
 *
 * @param di directory information
 * @param fn filename
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* FlacFile::Resolver::createFile(const DirInfo* di,
																					const QString& fn) const
{
	if (fn.right(5).QCM_toLower() == ".flac")
		return new FlacFile(di, fn);
	else
		return 0;
}

/**
 * Get a list with all extensions supported by FlacFile.
 *
 * @return list of file extensions.
 */
QStringList FlacFile::Resolver::getSupportedFileExtensions() const
{
	return QStringList() << ".flac";
}

#endif // HAVE_FLAC
