/**
 * \file playlistcreator.cpp
 * Playlist creator.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#include "playlistcreator.h"
#include "playlistconfig.h"
#include "filelistitem.h"
#include "taggedfile.h"
#include "dirinfo.h"
#include "importtrackdata.h"
#include "kid3.h"
#include "qtcompatmac.h"
#include <qdir.h>
#include <qurl.h>
#include <qfile.h>
#include <qtextstream.h>

/**
 * Constructor.
 *
 * @param topLevelDir top-level directory of playlist
 * @param cfg         playlist configuration
 */
PlaylistCreator::PlaylistCreator(const QString& topLevelDir,
																 const PlaylistConfig& cfg) :
	m_cfg(cfg)
{
	if (m_cfg.m_location == PlaylistConfig::PL_TopLevelDirectory) {
		m_playlistDirName = topLevelDir;
		if (!m_playlistDirName.endsWith(QChar(QDir::separator()))) {
			m_playlistDirName += QDir::separator();
		}
	}
}

/**
 * Write playlist containing added Entry elements.
 *
 * @return true if ok.
 */
bool PlaylistCreator::write()
{
	bool ok = true;
	if (!m_playlistFileName.isEmpty()) {
		QFile file(m_playlistDirName + m_playlistFileName);
		ok = file.open(QCM_WriteOnly);
		if (ok) {
			QTextStream stream(&file);

			switch (m_cfg.m_format) {
				case PlaylistConfig::PF_M3U:
					if (m_cfg.m_writeInfo) {
						stream << "#EXTM3U\n";
					}
					for (QMap<QString, Entry>::const_iterator it = m_entries.begin();
							 it != m_entries.end();
							 ++it) {
						if (m_cfg.m_writeInfo) {
							stream << QString("#EXTINF:%1,%2\n").
								arg((*it).duration).arg((*it).info);
						}
						stream << (*it).filePath << "\n";
					}
					break;
				case PlaylistConfig::PF_PLS:
				{
					unsigned nr = 1;
					stream << "[playlist]\n";
					stream << QString("NumberOfEntries=%1\n").arg(m_entries.size());
					for (QMap<QString, Entry>::const_iterator it = m_entries.begin();
							 it != m_entries.end();
							 ++it) {
						stream << QString("File%1=%2\n").arg(nr).arg((*it).filePath);
						if (m_cfg.m_writeInfo) {
							stream << QString("Title%1=%2\n").arg(nr).arg((*it).info);
							stream << QString("Length%1=%2\n").arg(nr).arg((*it).duration);
						}
						++nr;
					}
					stream << "Version=2\n";
				}
				break;
				case PlaylistConfig::PF_XSPF:
				{
					stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
					QString line = "<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\"";
					if (!m_cfg.m_useFullPath) {
						QUrl url(m_playlistDirName);
						url.QCM_setScheme("file");
						line += QString(" xml:base=\"%1\"").arg(
#if QT_VERSION >= 0x040000
							url.toEncoded().data()
#else
							url.toString(true, true)
#endif
							);
					}
					line += ">\n";
					stream << line;
					stream << "  <trackList>\n";

					for (QMap<QString, Entry>::const_iterator it = m_entries.begin();
							 it != m_entries.end();
							 ++it) {
						stream << "    <track>\n";
						QUrl url((*it).filePath);
						if (m_cfg.m_useFullPath) {
							url.QCM_setScheme("file");
						}
						stream << QString("      <location>%1</location>\n").arg(
#if QT_VERSION >= 0x040000
							url.toEncoded().data()
#else
							url.toString(true, m_cfg.m_useFullPath)
#endif
							);
						if (m_cfg.m_writeInfo) {
							// the info is already formatted in the case of XSPF
							stream << (*it).info;
						}
						stream << "    </track>\n";
					}

					stream << "  </trackList>\n";
					stream << "</playlist>\n";
				}
				break;
			}

			m_entries.clear();
			m_playlistFileName = "";
			file.close();
		}
	}
	return ok;
}


/**
 * Constructor.
 *
 * @param item item in file list
 * @param ctr  associated playlist creator
 */
PlaylistCreator::Item::Item(FileListItem* item, PlaylistCreator& ctr) :
		m_ctr(ctr), m_item(item), m_dirInfo(item->getDirInfo()),
		m_taggedFile(item->getFile()), m_trackData(0)
{
	QChar separator = QDir::separator();
	if (m_dirInfo) {
		m_dirName = m_dirInfo->getDirname();
	} else if (m_taggedFile) {
		m_dirName = m_taggedFile->getDirname();
		if (!m_dirName.endsWith(separator)) {
			m_dirName += separator;
		}
	}
	// fix double separators
	m_dirName.replace(QString(separator) + separator, separator);
}

/**
 * Destructor.
 */
PlaylistCreator::Item::~Item()
{
	delete m_trackData;
}

/**
 * Format string using tags and properties of item.
 *
 * @param format format string
 *
 * @return string with percent codes replaced.
 */
QString PlaylistCreator::Item::formatString(const QString& format)
{
	if (!m_trackData) {
		m_taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
		m_taggedFile = Kid3App::readWithTagLibIfId3V24(m_item, m_taggedFile);
#endif
		m_trackData = new ImportTrackData(*m_taggedFile);
	}
	return m_trackData->formatString(
		format,
		m_taggedFile->getDirInfo() ? m_taggedFile->getDirInfo()->getNumFiles() : 0);
}

/**
 * Add item to playlist.
 * This operation will write a playlist if the configuration is set to write
 * a playlist in every directory and a new directory is entered.
 *
 * @return true if ok.
 */
bool PlaylistCreator::Item::add()
{
	bool ok = true;
	if (m_ctr.m_cfg.m_location != PlaylistConfig::PL_TopLevelDirectory) {
		if (m_ctr.m_playlistDirName != m_dirName) {
			ok = m_ctr.write();
			m_ctr.m_playlistDirName = m_dirName;
		}
	}
	if (m_ctr.m_playlistFileName.isEmpty()) {
		if (!m_ctr.m_cfg.m_useFileNameFormat) {
			m_ctr.m_playlistFileName = QDir(m_ctr.m_playlistDirName).dirName();
		} else {
			m_ctr.m_playlistFileName = formatString(m_ctr.m_cfg.m_fileNameFormat);
		}
		switch (m_ctr.m_cfg.m_format) {
			case PlaylistConfig::PF_M3U:
				m_ctr.m_playlistFileName += ".m3u";
				break;
			case PlaylistConfig::PF_PLS:
				m_ctr.m_playlistFileName += ".pls";
				break;
			case PlaylistConfig::PF_XSPF:
				m_ctr.m_playlistFileName += ".xspf";
				break;
		}
	}
	QString filePath = m_dirName + m_taggedFile->getFilename();
	if (!m_ctr.m_cfg.m_useFullPath &&
			filePath.startsWith(m_ctr.m_playlistDirName)) {
		filePath = filePath.mid(m_ctr.m_playlistDirName.length());
	}
	QString sortKey;
	if (m_ctr.m_cfg.m_useSortTagField) {
		sortKey = formatString(m_ctr.m_cfg.m_sortTagField);
	}
	sortKey += filePath;
	PlaylistCreator::Entry entry;
	entry.filePath = filePath;
	if (m_ctr.m_cfg.m_writeInfo) {
		if (m_ctr.m_cfg.m_format != PlaylistConfig::PF_XSPF) {
			entry.info = formatString(m_ctr.m_cfg.m_infoFormat);
		} else {
			entry.info = formatString(
				"      <title>%{title}</title>\n"
				"      <creator>%{artist}</creator>\n"
				"      <album>%{album}</album>\n"
				"      <trackNum>%{track.1}</trackNum>\n"
				"      <duration>%{seconds}000</duration>\n");
		}
		TaggedFile::DetailInfo detailInfo;
		m_taggedFile->getDetailInfo(detailInfo);
		entry.duration = detailInfo.duration;
	} else {
		entry.info = QString();
		entry.duration = 0;
	}
	m_ctr.m_entries.insert(sortKey, entry);
	return ok;
}
