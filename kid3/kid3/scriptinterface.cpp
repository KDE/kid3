/**
 * \file scriptinterface.cpp
 * D-Bus script adaptor.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Dec 2007
 *
 * Copyright (C) 2007-2008  Urs Fleisch
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

#include "scriptinterface.h"
#ifdef HAVE_QTDBUS
#include <QDBusMessage>
#include <QDBusConnection>
#include <QFileInfo>
#include "kid3.h"
#include "id3form.h"
#include "filelistitem.h"
#include "taggedfile.h"
#include "frametable.h"
#include "filefilter.h"
#include "pictureframe.h"

/**
 * Constructor.
 *
 * @param parent parent object
 */
ScriptInterface::ScriptInterface(Kid3App* parent) :
	QDBusAbstractAdaptor(parent), m_app(parent)
{
	setAutoRelaySignals(true);
}

/**
 * Destructor.
 */
ScriptInterface::~ScriptInterface()
{
}

/**
 * Open file or directory.
 *
 * @param path path to file or directory
 *
 * @return true if ok.
 */
bool ScriptInterface::openDirectory(const QString& path)
{
	return m_app->openDirectory(path, false, true);
}

/**
 * Save all modified files.
 *
 * @return true if ok,
 *         else the error message is available using getErrorMessage().
 */
bool ScriptInterface::save()
{
	if (m_app->saveDirectory(true, &m_errorMsg)) {
		m_errorMsg.clear();
		return true;
	} else {
		m_errorMsg = "Error while writing file:\n" + m_errorMsg;
		return false;
	}
}

/**
 * Get a detailed error message provided by some methods.
 *
 * @return detailed error message.
 */
QString ScriptInterface::getErrorMessage() const
{
	return m_errorMsg;
}

/**
 * Revert changes in the selected files.
 */
void ScriptInterface::revert()
{
	m_app->slotFileRevert();
}

/**
 * Import tags from a file.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param path    path of file
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool ScriptInterface::importFromFile(int tagMask, const QString& path, int fmtIdx)
{
	return m_app->importTags(tagMask, path, fmtIdx);
}

/**
 * Download album cover art into the picture frame of the selected files.
 *
 * @param url           URL of picture file or album art resource
 * @param allFilesInDir true to add the image to all files in the directory
 */
void ScriptInterface::downloadAlbumArt(const QString& url, bool allFilesInDir)
{
	m_app->downloadImage(url, allFilesInDir);
}

/**
 * Export tags to a file.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param path    path of file
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool ScriptInterface::exportToFile(int tagMask, const QString& path, int fmtIdx)
{
	return m_app->exportTags(tagMask, path, fmtIdx);
}

/**
 * Create a playlist.
 *
 * @return true if ok.
 */
bool ScriptInterface::createPlaylist()
{
	return m_app->slotCreatePlaylist();
}

/**
 * Quit the application.
 */
void ScriptInterface::quit()
{
	selectAll();
	revert();
	m_app->slotFileQuit();
}

/**
 * Select all files.
 */
void ScriptInterface::selectAll()
{
	m_app->m_view->selectAllFiles();
}

/**
 * Deselect all files.
 */
void ScriptInterface::deselectAll()
{
	m_app->m_view->deselectAllFiles();
}

/**
 * Select the first file.
 *
 * @return true if there is a first file.
 */
bool ScriptInterface::firstFile()
{
	return m_app->m_view->selectFirstFile();
}

/**
 * Select the previous file.
 *
 * @return true if there is a previous file.
 */
bool ScriptInterface::previousFile()
{
	return m_app->m_view->selectPreviousFile();
}

/**
 * Select the next file.
 *
 * @return true if there is a next file.
 */
bool ScriptInterface::nextFile()
{
	return m_app->m_view->selectNextFile();
}

/**
 * Expand the current file item if it is a directory.
 * A file list item is a directory if getFileName() returns a name with
 * '/' as the last character.
 *
 * @return true if current file item is a directory.
 */
bool ScriptInterface::expandDirectory()
{
	FileListItem* item = m_app->m_view->currentFile();
	if (item) {
		if (item->getDirInfo()) {
			item->setExpanded(true);
			return true;
		}
	}
	return false;
}

/**
 * Apply the file name format.
 */
void ScriptInterface::applyFilenameFormat()
{
	m_app->slotApplyFilenameFormat();
}

/**
 * Apply the tag format.
 */
void ScriptInterface::applyTagFormat()
{
	m_app->slotApplyId3Format();
}

/**
 * Set the directory name from the tags.
 *
 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
 * @param format  directory name format
 * @param create  true to create, false to rename
 *
 * @return true if ok,
 *         else the error message is available using getErrorMessage().
 */
bool ScriptInterface::setDirNameFromTag(int tagMask, const QString& format,
																		bool create)
{
	if (m_app->renameDirectory(tagMask, format, create, &m_errorMsg)) {
		m_errorMsg.clear();
		return true;
	} else {
		m_errorMsg = "Error while renaming:\n" + m_errorMsg;
		return false;
	}
}

/**
 * Set subsequent track numbers in the selected files.
 *
 * @param tagMask      tag mask (bit 0 for tag 1, bit 1 for tag 2)
 * @param firstTrackNr number to use for first file
 */
void ScriptInterface::numberTracks(int tagMask, int firstTrackNr)
{
	m_app->numberTracks(firstTrackNr, (tagMask & 1) != 0, (tagMask & 2) != 0);
}

/**
 * Filter the files.
 *
 * @param expression filter expression
 */
void ScriptInterface::filter(const QString& expression)
{
	FileFilter filter;
	filter.setFilterExpression(expression);
	filter.initParser();
	m_app->applyFilter(filter);
}

/**
 * Convert ID3v2.3 tags to ID3v2.4.
 */
void ScriptInterface::convertToId3v24()
{
#ifdef HAVE_TAGLIB
	m_app->slotConvertToId3v24();
#endif
}

/**
 * Convert ID3v2.4 tags to ID3v2.3.
 */
void ScriptInterface::convertToId3v23()
{
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
	m_app->slotConvertToId3v23();
#endif
}

/**
 * Get path of directory.
 *
 * @return absolute path of directory.
 */
QString ScriptInterface::getDirectoryName()
{
	const DirInfo* dirinfo = m_app->m_view->getDirInfo();
	return dirinfo ? dirinfo->getDirname() : "";
}

/**
 * Get name of current file.
 *
 * @return absolute file name, ends with "/" if it is a directory.
 */
QString ScriptInterface::getFileName()
{
	FileListItem* item = m_app->m_view->currentFile();
	if (item) {
		const DirInfo* dirinfo = item->getDirInfo();
		if (dirinfo) {
			QString dirname = dirinfo->getDirname();
			if (!dirname.endsWith('/')) dirname += '/';
			return dirname;
		} else {
			TaggedFile* taggedFile = item->getFile();
			if (taggedFile) {
				return taggedFile->getAbsFilename();
			}
		}
	}
	return "";
}

/**
 * Set name of selected file.
 * The file will be renamed when the directory is saved.
 *
 * @param name file name.
 */
void ScriptInterface::setFileName(const QString& name)
{
	QFileInfo fi(name);
	m_app->m_view->setFilename(fi.fileName());
}

/**
 * Set format to use when setting the filename from the tags.
 *
 * @param format file name format
 * @see setFileNameFromTag()
 */
void ScriptInterface::setFileNameFormat(const QString& format)
{
	m_app->m_view->setFilenameFormat(format);
}

/**
 * Set the file names of the selected files from the tags.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @see setFileNameFormat()
 */
void ScriptInterface::setFileNameFromTag(int tagMask)
{
	if (tagMask == 1 || tagMask == 2) {
		m_app->getFilenameFromTags(tagMask);
	}
}

/**
 * Get value of frame.
 * To get binary data like a picture, the name of a file to write can be
 * added after the @a name, e.g. "Picture:/path/to/file".
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param name    name of frame (e.g. "Artist")
 */
QString ScriptInterface::getFrame(int tagMask, const QString& name)
{
	QString frameName(name);
	QString dataFileName;
	int colonIndex = frameName.indexOf(':');
	if (colonIndex != -1) {
		dataFileName = frameName.mid(colonIndex + 1);
		frameName.truncate(colonIndex);
	}
	FrameTable* ft = (tagMask & 2) ? m_app->m_view->frameTableV2() :
		m_app->m_view->frameTableV1();
	ft->tableToFrames();
	FrameCollection::iterator it = ft->frames().findByName(frameName);
	if (it != ft->frames().end()) {
		if (!dataFileName.isEmpty()) {
			PictureFrame::writeDataToFile(*it, dataFileName);
		}
		return it->getValue();
	} else {
		return "";
	}
}

/**
 * Set value of frame.
 * For tag 2 (@a tagMask 2), if no frame with @a name exists, a new frame
 * is added, if @a value is empty, the frame is deleted.
 * To add binary data like a picture, a file can be added after the
 * @a name, e.g. "Picture:/path/to/file".
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param name    name of frame (e.g. "Artist")
 * @param value   value of frame
 */
bool ScriptInterface::setFrame(int tagMask, const QString& name,
													 const QString& value)
{
	QString frameName(name);
	QString dataFileName;
	int colonIndex = frameName.indexOf(':');
	if (colonIndex != -1) {
		dataFileName = frameName.mid(colonIndex + 1);
		frameName.truncate(colonIndex);
	}
	FrameTable* ft = (tagMask & 2) ? m_app->m_view->frameTableV2() :
		m_app->m_view->frameTableV1();
	ft->tableToFrames();
	FrameCollection::iterator it = ft->frames().findByName(frameName);
	if (it != ft->frames().end()) {
		if (it->getType() == Frame::FT_Picture && !dataFileName.isEmpty() &&
				(tagMask & 2) != 0) {
			m_app->deleteFrame(it->getName());
			PictureFrame frame;
			PictureFrame::setDescription(frame, value);					
			PictureFrame::setDataFromFile(frame, dataFileName);
			PictureFrame::setMimeTypeFromFileName(frame, dataFileName);
			m_app->addFrame(&frame);
		} else if (value.isEmpty() && (tagMask & 2) != 0) {
			m_app->deleteFrame(it->getName());
		} else {
			Frame& frame = const_cast<Frame&>(*it);
			frame.setValueIfChanged(value);
			ft->framesToTable();
		}
		return true;
	} else if (tagMask & 2) {
		Frame::Type type = Frame::getTypeFromName(frameName);
		Frame frame(type, value, frameName, -1);
		if (type == Frame::FT_Picture && !dataFileName.isEmpty()) {
			PictureFrame::setFields(frame);
			PictureFrame::setDescription(frame, value);					
			PictureFrame::setDataFromFile(frame, dataFileName);
			PictureFrame::setMimeTypeFromFileName(frame, dataFileName);
		}
		m_app->addFrame(&frame);
		return true;
	}
	return false;
}

/**
 * Get all frames of a tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 *
 * @return list with alternating frame names and values.
 */
QStringList ScriptInterface::getTag(int tagMask)
{
	QStringList lst;
	FrameTable* ft = (tagMask & 2) ? m_app->m_view->frameTableV2() :
		m_app->m_view->frameTableV1();
	ft->tableToFrames();
	for (FrameCollection::const_iterator it = ft->frames().begin();
			 it != ft->frames().end();
			 ++it) {
		lst << it->getName();
		lst << it->getValue();
	}
	return lst;
}

/**
 * Get technical information about file.
 * Properties are Format, Bitrate, Samplerate, Channels, Duration,
 * Channel Mode, VBR, Tag 1, Tag 2.
 * Properties which are not available are omitted.
 *
 * @return list with alternating property names and values.
 */
QStringList ScriptInterface::getInformation()
{
	QStringList lst;
	FileListItem* item = m_app->m_view->currentFile();
	if (item) {
		TaggedFile* taggedFile = item->getFile();
		if (taggedFile) {
			TaggedFile::DetailInfo info;
			taggedFile->getDetailInfo(info);
			if (info.valid) {
				lst << "Format" << info.format;
				if (info.bitrate > 0 && info.bitrate < 999) {
					lst << "Bitrate" << QString::number(info.bitrate);
				}
				if (info.sampleRate > 0) {
					lst << "Samplerate" << QString::number(info.sampleRate);
				}
				if (info.channels > 0) {
					lst << "Channels" << QString::number(info.channels);
				}
				if (info.duration > 0) {
					lst << "Duration" << QString::number(info.duration);
				}
				if (info.channelMode == TaggedFile::DetailInfo::CM_Stereo ||
						info.channelMode == TaggedFile::DetailInfo::CM_JointStereo) {
					lst << "Channel Mode" <<
						(info.channelMode == TaggedFile::DetailInfo::CM_Stereo ?
						 "Stereo" : "Joint Stereo");
				}
				if (info.vbr) {
					lst << "VBR" << "1";
				}
			}
			QString tag1 = taggedFile->getTagFormatV1();
			if (!tag1.isEmpty()) {
				lst << "Tag 1" << tag1;
			}
			QString tag2 = taggedFile->getTagFormatV2();
			if (!tag2.isEmpty()) {
				lst << "Tag 2" << tag2;
			}
		}
	}
	return lst;
}

/**
 * Set tag from file name.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::setTagFromFileName(int tagMask)
{
	if (tagMask & 1) {
		m_app->getTagsFromFilenameV1();
	} else if (tagMask & 2) {
		m_app->getTagsFromFilenameV2();
	}
}

/**
 * Set tag from other tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::setTagFromOtherTag(int tagMask)
{
	if (tagMask & 1) {
		m_app->copyV2ToV1();
	} else if (tagMask & 2) {
		m_app->copyV1ToV2();
	}
}

/**
 * Copy tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::copyTag(int tagMask)
{
	if (tagMask & 1) {
		m_app->copyTagsV1();
	} else if (tagMask & 2) {
		m_app->copyTagsV2();
	}
}

/**
 * Paste tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::pasteTag(int tagMask)
{
	if (tagMask & 1) {
		m_app->pasteTagsV1();
	} else if (tagMask & 2) {
		m_app->pasteTagsV2();
	}
}

/**
 * Remove tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::removeTag(int tagMask)
{
	if (tagMask & 1) {
		m_app->removeTagsV1();
	} else if (tagMask & 2) {
		m_app->removeTagsV2();
	}
}

/**
 * Hide or show tag in GUI.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param hide    true to hide tag
 */
void ScriptInterface::hideTag(int tagMask, bool hide)
{
	if (tagMask & 1) {
		m_app->m_view->hideV1(hide);
	} else if (tagMask & 2) {
		m_app->m_view->hideV2(hide);
	}
}

/**
 * Reparse the configuration.
 * Automated configuration changes are possible by modifying
 * the configuration file and then reparsing the configuration.
 */
void ScriptInterface::reparseConfiguration()
{
	m_app->readOptions();
}

#else // HAVE_QTDBUS

ScriptInterface::ScriptInterface(Kid3App*) {}
ScriptInterface::~ScriptInterface() {}
bool ScriptInterface::openDirectory(const QString&) { return false; }
bool ScriptInterface::save() { return false; }
QString ScriptInterface::getErrorMessage() const { return ""; }
void ScriptInterface::revert() {}
bool ScriptInterface::importFromFile(int, const QString&, int) { return false; }
void ScriptInterface::downloadAlbumArt(const QString&, bool) {}
bool ScriptInterface::exportToFile(int, const QString&, int) { return false; }
bool ScriptInterface::createPlaylist() { return false; }
void ScriptInterface::quit() {}
void ScriptInterface::selectAll() {}
void ScriptInterface::deselectAll() {}
bool ScriptInterface::firstFile() { return false; }
bool ScriptInterface::previousFile() { return false; }
bool ScriptInterface::nextFile() { return false; }
bool ScriptInterface::expandDirectory() { return false; }
void ScriptInterface::applyFilenameFormat() {}
void ScriptInterface::applyTagFormat() {}
bool ScriptInterface::setDirNameFromTag(int, const QString&, bool) { return false; }
void ScriptInterface::numberTracks(int, int) {}
void ScriptInterface::filter(const QString&) {}
void ScriptInterface::convertToId3v24() {}
void ScriptInterface::convertToId3v23() {}
QString ScriptInterface::getDirectoryName() { return ""; }
QString ScriptInterface::getFileName() { return ""; }
void ScriptInterface::setFileName(const QString&) {}
void ScriptInterface::setFileNameFormat(const QString&) {}
void ScriptInterface::setFileNameFromTag(int) {}
QString ScriptInterface::getFrame(int, const QString&) { return ""; }
bool ScriptInterface::setFrame(int, const QString&, const QString&) { return false; }
QStringList ScriptInterface::getTag(int) { return QStringList(); }
QStringList ScriptInterface::getInformation() { return QStringList(); }
void ScriptInterface::setTagFromFileName(int) {}
void ScriptInterface::setTagFromOtherTag(int) {}
void ScriptInterface::copyTag(int) {}
void ScriptInterface::pasteTag(int) {}
void ScriptInterface::removeTag(int) {}
void ScriptInterface::hideTag(int, bool) {}
void ScriptInterface::reparseConfiguration() {}

#endif // HAVE_QTDBUS
