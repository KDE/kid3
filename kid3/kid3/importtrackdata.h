/**
 * \file importtrackdata.h
 * Track data used for import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 7 Jul 2005
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

#ifndef IMPORTTRACKDATA_H
#define IMPORTTRACKDATA_H

#include "standardtags.h"
#include <qglobal.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QVector>
#else
#include <qvaluevector.h>
#endif

/**
 * Track data used for import.
 */
class ImportTrackData : public StandardTags {
public:
	/**
	 * Constructor.
	 * @param absFilename  absolute filename
	 * @param fileDuration duration in seconds
	 */
	explicit ImportTrackData(const QString& absFilename = QString::null,
													 int fileDuration = 0) :
		m_fileDuration(fileDuration), m_importDuration(0),
		m_absFilename(absFilename) {}

	/**
	 * Destructor.
	 */
	~ImportTrackData() {}

	/**
	 * Get duration of file.
	 * @return duration of file.
	 */
	int getFileDuration() const { return m_fileDuration; }

	/**
	 * Set duration of file.
	 * @param duration duration of file
	 */
	void setFileDuration(int duration) { m_fileDuration = duration; }

	/**
	 * Get duration of import.
	 * @return duration of import.
	 */
	int getImportDuration() const { return m_importDuration; }

	/**
	 * Set duration of import.
	 * @param duration duration of import
	 */
	void setImportDuration(int duration) { m_importDuration = duration; }

	/**
	 * Set standard tag fields.
	 * @param st standard tags
	 */
	void setStandardTags(const StandardTags& st) {
		*(static_cast<StandardTags*>(this)) = st;
	}

	/**
	 * Get absolute filename.
	 *
	 * @return absolute file path.
	 */
	QString getAbsFilename() const { return m_absFilename; }

	/**
	 * Format a string from track data.
	 * Supported format fields:
	 * Those supported by StandardTags::formatString()
	 * %f filename
	 * %p path to file
	 * %u URL of file
	 * %d duration in minutes:seconds
	 * %D duration in seconds
	 *
	 * @param format format specification
	 *
	 * @return formatted string.
	 */
	QString formatString(const QString& format) const;

private:
	int m_fileDuration;
	int m_importDuration;
	QString m_absFilename;
};

/**
 * Vector containing tracks to import and artist, album names.
 */
class ImportTrackDataVector : public
#if QT_VERSION >= 0x040000
QVector<ImportTrackData>
#else
QValueVector<ImportTrackData>
#endif
{
public:
	QString m_artist; /**< album artist */
	QString m_album;  /**< album name */
};

#endif // IMPORTTRACKDATA_H
