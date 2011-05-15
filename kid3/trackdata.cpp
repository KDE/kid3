/**
 * \file trackdata.cpp
 * Track data, frames with association to tagged file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Feb 2007
 *
 * Copyright (C) 2007-2011  Urs Fleisch
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

#include "trackdata.h"
#include <QString>
#include <QUrl>
#include <QDir>
#include "fileproxymodel.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param trackData track data
 * @param numTracks number of tracks in album
 * @param str       string with format codes
 */
TrackDataFormatReplacer::TrackDataFormatReplacer(
	const TrackData& trackData, unsigned numTracks, const QString& str) :
	FrameFormatReplacer(trackData, str), m_trackData(trackData),
	m_numTracks(numTracks) {}

/**
 * Destructor.
 */
TrackDataFormatReplacer::~TrackDataFormatReplacer() {}

/**
 * Replace a format code (one character %c or multiple characters %{chars}).
 * Supported format fields:
 * Those supported by FrameFormatReplacer::getReplacement()
 * %f filename
 * %p path to file
 * %u URL of file
 * %d duration in minutes:seconds
 * %D duration in seconds
 * %n number of tracks
 *
 * @param code format code
 *
 * @return replacement string,
 *         QString::null if code not found.
 */
QString TrackDataFormatReplacer::getReplacement(const QString& code) const
{
	QString result = FrameFormatReplacer::getReplacement(code);
	if (result.isNull()) {
		QString name;

		if (code.length() == 1) {
			static const struct {
				char shortCode;
				const char* longCode;
			} shortToLong[] = {
				{ 'f', "file" },
				{ 'p', "filepath" },
				{ 'u', "url" },
				{ 'd', "duration" },
				{ 'D', "seconds" },
				{ 'n', "tracks" },
				{ 'e', "extension" },
				{ 'O', "tag1" },
				{ 'o', "tag2" },
				{ 'b', "bitrate" },
				{ 'v', "vbr" },
				{ 'r', "samplerate" },
				{ 'm', "mode" },
				{ 'h', "channels" },
				{ 'k', "codec" }
			};
			const char c = code[0].toLatin1();
			for (unsigned i = 0; i < sizeof(shortToLong) / sizeof(shortToLong[0]); ++i) {
				if (shortToLong[i].shortCode == c) {
					name = shortToLong[i].longCode;
					break;
				}
			}
		} else if (code.length() > 1) {
			name = code;
		}

		if (!name.isNull()) {
			TaggedFile::DetailInfo info;
			m_trackData.getDetailInfo(info);
			if (name == "file") {
				QString filename(m_trackData.getAbsFilename());
				int sepPos = filename.lastIndexOf('/');
				if (sepPos < 0) {
					sepPos = filename.lastIndexOf(QDir::separator());
				}
				if (sepPos >= 0) {
					filename.remove(0, sepPos + 1);
				}
				result = filename;
			} else if (name == "filepath") {
				result = m_trackData.getAbsFilename();
			} else if (name == "url") {
				QUrl url;
				url.setPath(m_trackData.getAbsFilename());
				url.setScheme("file");
				result = url.toString();
			} else if (name == "duration") {
				result = TaggedFile::formatTime(m_trackData.getFileDuration());
			} else if (name == "seconds") {
				result = QString::number(m_trackData.getFileDuration());
			} else if (name == "tracks") {
				result = QString::number(m_numTracks);
			} else if (name == "extension") {
				result = m_trackData.getFileExtension();
			} else if (name == "tag1") {
				result = m_trackData.getTagFormatV1();
			} else if (name == "tag2") {
				result = m_trackData.getTagFormatV2();
			} else if (name == "bitrate") {
				result.setNum(info.bitrate);
			} else if (name == "vbr") {
				result = info.vbr ? "VBR" : "";
			} else if (name == "samplerate") {
				result.setNum(info.sampleRate);
			} else if (name == "mode") {
				switch (info.channelMode) {
					case TaggedFile::DetailInfo::CM_Stereo:
						result = "Stereo";
						break;
					case TaggedFile::DetailInfo::CM_JointStereo:
						result = "Joint Stereo";
						break;
					case TaggedFile::DetailInfo::CM_None:
					default:
						result = "";
				}
			} else if (name == "channels") {
				result.setNum(info.channels);
			} else if (name == "codec") {
				result = info.format;
			}
		}
	}

	return result;
}

/**
 * Get help text for supported format codes.
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString TrackDataFormatReplacer::getToolTip(bool onlyRows)
{
	QString str;
	if (!onlyRows) str += "<table>\n";
	str += FrameFormatReplacer::getToolTip(true);

	str += "<tr><td>%f</td><td>%{file}</td><td>";
	str += QCM_translate("Filename");
	str += "</td></tr>\n";

	str += "<tr><td>%p</td><td>%{filepath}</td><td>";
	str += QCM_translate(I18N_NOOP("Absolute path to file"));
	str += "</td></tr>\n";

	str += "<tr><td>%u</td><td>%{url}</td><td>";
	str += QCM_translate("URL");
	str += "</td></tr>\n";

	str += "<tr><td>%d</td><td>%{duration}</td><td>";
	str += QCM_translate(I18N_NOOP("Length"));
	str += " &quot;M:S&quot;</td></tr>\n";

	str += "<tr><td>%D</td><td>%{seconds}</td><td>";
	str += QCM_translate(I18N_NOOP("Length"));
	str += " &quot;S&quot;</td></tr>\n";

	str += "<tr><td>%n</td><td>%{tracks}</td><td>";
	str += QCM_translate(I18N_NOOP("Number of tracks"));
	str += "</td></tr>\n";

	str += "<tr><td>%e</td><td>%{extension}</td><td>";
	str += QCM_translate(I18N_NOOP("Extension"));
	str += "</td></tr>\n";

	str += "<tr><td>%O</td><td>%{tag1}</td><td>";
	str += QCM_translate("Tag 1");
	str += "</td></tr>\n";

	str += "<tr><td>%o</td><td>%{tag2}</td><td>";
	str += QCM_translate("Tag 2");
	str += "</td></tr>\n";

	str += "<tr><td>%b</td><td>%{bitrate}</td><td>";
	str += QCM_translate(I18N_NOOP("Bitrate"));
	str += "</td></tr>\n";

	str += "<tr><td>%v</td><td>%{vbr}</td><td>";
	str += QCM_translate(I18N_NOOP("VBR"));
	str += "</td></tr>\n";

	str += "<tr><td>%r</td><td>%{samplerate}</td><td>";
	str += QCM_translate(I18N_NOOP("Samplerate"));
	str += "</td></tr>\n";

	str += "<tr><td>%m</td><td>%{mode}</td><td>Stereo, Joint Stereo</td></tr>\n";

	str += "<tr><td>%h</td><td>%{channels}</td><td>";
	str += QCM_translate(I18N_NOOP("Channels"));
	str += "</td></tr>\n";

	str += "<tr><td>%k</td><td>%{codec}</td><td>";
	str += QCM_translate(I18N_NOOP("Codec"));
	str += "</td></tr>\n";

	if (!onlyRows) str += "</table>\n";
	return str;
}


/**
 * Constructor.
 */
TrackData::TrackData()
{}

/**
 * Constructor.
 * All fields except the import duration are set from the tagged file,
 * which should be read using readTags() before.
 *
 * @param taggedFile tagged file providing track data
 * @param tagVersion source of frames
 */
TrackData::TrackData(TaggedFile& taggedFile, TagVersion tagVersion) :
	m_taggedFileIndex(taggedFile.getIndex())
{
	switch (tagVersion) {
	case TagV1:
		taggedFile.getAllFramesV1(*this);
		break;
	case TagV2:
		taggedFile.getAllFramesV2(*this);
		break;
	case TagV2V1:
	{
		FrameCollection framesV1;
		taggedFile.getAllFramesV1(framesV1);
		taggedFile.getAllFramesV2(*this);
		merge(framesV1);
		break;
	}
	case TagNone:
		;
	}
}

/**
 * Get tagged file associated with this track data.
 * @return tagged file, 0 if none assigned.
 */
TaggedFile* TrackData::getTaggedFile() const {
	return FileProxyModel::getTaggedFileOfIndex(m_taggedFileIndex);
}

/**
 * Get duration of file.
 * @return duration of file.
 */
int TrackData::getFileDuration() const
{
	TaggedFile* taggedFile = getTaggedFile();
	return taggedFile ? taggedFile->getDuration() : 0;
}

/**
 * Get absolute filename.
 *
 * @return absolute file path.
 */
QString TrackData::getAbsFilename() const
{
	TaggedFile* taggedFile = getTaggedFile();
	return taggedFile ? taggedFile->getAbsFilename() : QString();
}

/**
 * Get the format of tag 1.
 *
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TrackData::getTagFormatV1() const
{
	TaggedFile* taggedFile = getTaggedFile();
	return taggedFile ? taggedFile->getTagFormatV1() : QString();
}

/**
 * Get the format of tag 2.
 *
 * @return string describing format of tag 2,
 *         e.g. "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TrackData::getTagFormatV2() const
{
	TaggedFile* taggedFile = getTaggedFile();
	return taggedFile ? taggedFile->getTagFormatV2() : QString();
}

/**
 * Get detail info.
 * @param info the detail information is returned here
 */
void TrackData::getDetailInfo(TaggedFile::DetailInfo& info) const
{
	if (TaggedFile* taggedFile = getTaggedFile()) {
		taggedFile->getDetailInfo(info);
	}
}

/**
 * Format a string from track data.
 * Supported format fields:
 * Those supported by TrackDataFormatReplacer::getReplacement()
 *
 * @param format    format specification
 * @param numTracks number of tracks in album
 *
 * @return formatted string.
 */
QString TrackData::formatString(const QString& format, unsigned numTracks) const
{
	TrackDataFormatReplacer fmt(*this, numTracks, format);
	fmt.replaceEscapedChars();
	fmt.replacePercentCodes();
	return fmt.getString();
}

/**
 * Get help text for format codes supported by formatString().
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString TrackData::getFormatToolTip(bool onlyRows)
{
	return TrackDataFormatReplacer::getToolTip(onlyRows);
}

/**
 * Get file extension including the dot.
 *
 * @return file extension, e.g. ".mp3".
 */
QString TrackData::getFileExtension() const
{
	QString fileExtension;
	QString absFilename;
	if (TaggedFile* taggedFile = getTaggedFile()) {
		fileExtension = taggedFile->getFileExtension();
		absFilename = taggedFile->getAbsFilename();
	}
	if (!fileExtension.isEmpty()) {
		return fileExtension;
	} else {
		int dotPos = absFilename.lastIndexOf(".");
		return dotPos != -1 ? absFilename.mid(dotPos) : QString();
	}
}


/**
 * Clear vector and associated data.
 */
void ImportTrackDataVector::clearData()
{
	clear();
	m_coverArtUrl = QString();
}

/**
 * Get album artist.
 * @return album artist.
 */
QString ImportTrackDataVector::getArtist() const
{
	return getFrame(Frame::FT_Artist);
}

/**
 * Get album title.
 * @return album title.
 */
QString ImportTrackDataVector::getAlbum() const
{
	return getFrame(Frame::FT_Album);
}

/**
 * Check if tag 1 is supported in the first track.
 * @return true if tag 1 is supported.
 */
bool ImportTrackDataVector::isTagV1Supported() const
{
	if (!isEmpty()) {
		TaggedFile* taggedFile = at(0).getTaggedFile();
		if (taggedFile) {
			return taggedFile->isTagV1Supported();
		}
	}
	return true;
}

/**
 * Get frame from first track.
 * @param type frame type
 * @return value of frame.
 */
QString ImportTrackDataVector::getFrame(Frame::Type type) const
{
	QString result;
	if (!isEmpty()) {
		const ImportTrackData& trackData = at(0);
		result = trackData.getValue(type);
		if (!result.isEmpty())
			return result;
		TaggedFile* taggedFile = trackData.getTaggedFile();
		FrameCollection frames;
		taggedFile->getAllFramesV2(frames);
		result = frames.getValue(type);
		if (!result.isEmpty())
			return result;
		taggedFile->getAllFramesV1(frames);
		result = frames.getValue(type);
	}
	return result;
}
