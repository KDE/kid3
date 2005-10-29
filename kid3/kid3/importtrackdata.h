/**
 * \file importtrackdata.h
 * Track data used for import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 7 Jul 2005
 */

#ifndef IMPORTTRACKDATA_H
#define IMPORTTRACKDATA_H

#include "standardtags.h"
#include <qglobal.h>
#if QT_VERSION >= 300
#include <qvaluevector.h>
#else
#include <qvaluelist.h>
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
	QString getAbsFilename(void) const { return m_absFilename; }

private:
	int m_fileDuration;
	int m_importDuration;
	QString m_absFilename;
};

#if QT_VERSION >= 300
typedef QValueVector<ImportTrackData> ImportTrackDataVector;
#else
typedef QValueList<ImportTrackData> ImportTrackDataVector;
#endif

#endif // IMPORTTRACKDATA_H
