/**
 * \file importtrackdata.cpp
 * Track data used for import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Feb 2007
 *
 * Copyright (C) 2007  Urs Fleisch
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
#include "taggedfile.h"
#include <qstring.h>
#include <qurl.h>
#include <qdir.h>

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
QString ImportTrackData::formatString(const QString& format) const
{
	QString fmt = StandardTags::formatString(format);
	if (!fmt.isEmpty()) {
		const int numTagCodes = 5;
		const QChar tagCode[numTagCodes] = {
	    'f', 'p', 'u', 'd', 'D'
		};
		const QStringList tagLongCodes =
			(QStringList() << "file" << "filepath" << "url" <<
			 "duration" << "seconds");
		QString tagStr[numTagCodes];

		QString filename(getAbsFilename());
		int sepPos = filename.QCM_lastIndexOf('/');
		if (sepPos < 0) {
			sepPos = filename.QCM_lastIndexOf(QDir::separator());
		}
		if (sepPos >= 0) {
			filename.remove(0, sepPos + 1);
		}

		tagStr[0] = filename;
		tagStr[1] = getAbsFilename();
		QUrl url;
		url.QCM_setPath(tagStr[1]);
		url.QCM_setScheme("file");
		tagStr[2] = url.toString(
#if QT_VERSION < 0x040000
			true
#endif
			);
		tagStr[3] = TaggedFile::formatTime(getFileDuration());
		tagStr[4] = QString::number(getFileDuration());

		fmt = replaceEscapedChars(fmt);
		fmt = replacePercentCodes(fmt, tagCode, tagLongCodes, tagStr, numTagCodes);
	}
	return fmt;
}

/**
 * Get help text for format codes supported by formatString().
 *
 * @param onlyRows if true only the <tr> elements are returned,
 *                 not the surrounding <table>
 *
 * @return help text.
 */
QString ImportTrackData::getFormatToolTip(bool onlyRows)
{
	QString str;
	if (!onlyRows) str += "<table>\n";
	str += StandardTags::getFormatToolTip(true);

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
	str += QCM_translate("Length");
	str += " &quot;M:S&quot;</td></tr>\n";

	str += "<tr><td>%D</td><td>%{seconds}</td><td>";
	str += QCM_translate("Length");
	str += " &quot;S&quot;</td></tr>\n";

	if (!onlyRows) str += "</table>\n";
	return str;
}
