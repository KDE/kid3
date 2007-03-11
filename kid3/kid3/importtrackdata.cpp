/**
 * \file importtrackdata.cpp
 * Track data used for import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Feb 2007
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
		QString tagStr[numTagCodes];

		QString filename(getAbsFilename());
		int sepPos = filename.findRev('/');
		if (sepPos < 0) {
			sepPos = filename.findRev(QDir::separator());
		}
		if (sepPos >= 0) {
			filename.remove(0, sepPos + 1);
		}

		tagStr[0] = filename;
		tagStr[1] = getAbsFilename();
		QUrl url;
		url.setFileName(tagStr[1]);
		url.setProtocol("file");
		tagStr[2] = url.toString(
#if QT_VERSION < 0x040000
			true
#endif
			);
		tagStr[3] = TaggedFile::formatTime(getFileDuration());
		tagStr[4] = QString::number(getFileDuration());

		fmt = replaceEscapedChars(fmt);
		fmt = replacePercentCodes(fmt, tagCode, tagStr, numTagCodes);
	}
	return fmt;
}
