/**
 * \file importdialog.h
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kdialogbase.h>
#else
#include <qdialog.h>
#endif

class ImportSelector;

/**
 * Import dialog.
 */
class ImportDialog : public
#ifdef CONFIG_USE_KDE
KDialogBase
#else
QDialog
#endif
{
public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param caption dialog title
	 */
	ImportDialog(QWidget *parent, QString &caption);
	/**
	 * Destructor.
	 */
	~ImportDialog();
	/**
	 * Look for album specific information (artist, album, year, genre) in
	 * a header (e.g. in a freedb header).
	 *
	 * @param st standard tags to put resulting values in,
	 *           fields which are not found are not touched.
	 *
	 * @return true if one or more field were found.
	 */
	bool parseHeader(StandardTags &st);
	/**
	 * Get next line as standardtags from imported file or clipboard.
	 *
	 * @param st standard tags
	 * @param start true to start with the first line, false for all
	 *              other lines
	 *
	 * @return true if ok (result in st),
	 *         false if end of file reached.
	 */
	bool getNextTags(StandardTags &st, bool start = false);
	/**
	 * Set ID3v1 or ID3v2 tags as destination.
	 *
	 * @param v1 true to set ID3v1, false for ID3v2
	 */
	void setDestV1(bool v1);
	/**
	 * Get import destination.
	 *
	 * @return true if ID3v1 is destination,
	 *         false if ID3v2.
	 */
	bool getDestV1() const;
	/**
	 * Set import format regexp.
	 *
	 * @param names   import format names list
	 * @param headers import format header regexps
	 * @param tracks  import format track regexps
	 * @param idx     selected index
	 */
	void setImportFormat(const QStringList &names,
						 const QStringList &headers,
						 const QStringList &tracks,
						 int idx);
	/**
	 * Get import format regexp.
	 *
	 * @param name   import format name
	 * @param header import format header regexp
	 * @param track  import format track regexp
	 *
	 * @return index of current selection.
	 */
	int getImportFormat(QString &name,
						QString &header,
						QString &track) const;

private:
	/** import selector widget */
	ImportSelector *impsel;
};

#endif
