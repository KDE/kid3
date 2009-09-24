/**
 * \file importtrackdata.cpp
 * Track data used for import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Feb 2007
 *
 * Copyright (C) 2007-2009  Urs Fleisch
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

#include "importtrackdata.h"
#include <qstring.h>
#include <qurl.h>
#include <qdir.h>

/**
 * Constructor.
 *
 * @param trackData track data
 * @param numTracks number of tracks in album
 * @param str       string with format codes
 */
TrackDataFormatReplacer::TrackDataFormatReplacer(
	const ImportTrackData& trackData, unsigned numTracks, const QString& str) :
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
#if QT_VERSION >= 0x040000
			const char c = code[0].toLatin1();
#else
			const char c = code[0].latin1();
#endif
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
			if (name == "file") {
				QString filename(m_trackData.getAbsFilename());
				int sepPos = filename.QCM_lastIndexOf('/');
				if (sepPos < 0) {
					sepPos = filename.QCM_lastIndexOf(QDir::separator());
				}
				if (sepPos >= 0) {
					filename.remove(0, sepPos + 1);
				}
				result = filename;
			} else if (name == "filepath") {
				result = m_trackData.getAbsFilename();
			} else if (name == "url") {
				QUrl url;
				url.QCM_setPath(m_trackData.getAbsFilename());
				url.QCM_setScheme("file");
				result = url.toString(
#if QT_VERSION < 0x040000
					true
#endif
					);
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
				result.setNum(m_trackData.getDetailInfo().bitrate);
			} else if (name == "vbr") {
				result = m_trackData.getDetailInfo().vbr ? "VBR" : "";
			} else if (name == "samplerate") {
				result.setNum(m_trackData.getDetailInfo().sampleRate);
			} else if (name == "mode") {
				switch (m_trackData.getDetailInfo().channelMode) {
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
				result.setNum(m_trackData.getDetailInfo().channels);
			} else if (name == "codec") {
				result = m_trackData.getDetailInfo().format;
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
 * All fields except the import duration are set from the tagged file,
 * which should be read using readTags() before. The frames are merged
 * from tag 2 and tag 1 (where tag 2 is not set).
 *
 * @param taggedFile tagged file providing track data
 */
ImportTrackData::ImportTrackData(TaggedFile& taggedFile) :
	m_fileDuration(taggedFile.getDuration()),
	m_importDuration(0),
	m_absFilename(taggedFile.getAbsFilename()),
	m_fileExtension(taggedFile.getFileExtension()),
	m_tagFormatV1(taggedFile.getTagFormatV1()),
	m_tagFormatV2(taggedFile.getTagFormatV2())
{
	taggedFile.getDetailInfo(m_detailInfo);
	FrameCollection framesV1;
	taggedFile.getAllFramesV1(framesV1);
	taggedFile.getAllFramesV2(*this);
	merge(framesV1);
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
QString ImportTrackData::formatString(const QString& format, unsigned numTracks) const
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
QString ImportTrackData::getFormatToolTip(bool onlyRows)
{
	return TrackDataFormatReplacer::getToolTip(onlyRows);
}

/**
 * Get file extension including the dot.
 *
 * @return file extension, e.g. ".mp3".
 */
QString ImportTrackData::getFileExtension() const
{
	if (!m_fileExtension.isEmpty()) {
		return m_fileExtension;
	} else {
		int dotPos = m_absFilename.QCM_lastIndexOf(".");
		return dotPos != -1 ? m_absFilename.mid(dotPos) : QString();
	}
}
