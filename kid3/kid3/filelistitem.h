/**
 * \file filelistitem.h
 * Item in filelist.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 02 Oct 2006
 */

#ifndef FILELISTITEM_H
#define FILELISTITEM_H

#include <qlistbox.h>
#include <qpixmap.h>

class QPainter;
class TaggedFile;

/** List box item containing tagged file */
class FileListItem : public QListBoxItem {
public:
	/**
	 * Constructor.
	 *
	 * @param file tagged file (will be owned by this item)
	 */
	FileListItem(TaggedFile* file);

	/**
	 * Destructor.
	 */
	virtual ~FileListItem();

	/**
	 * Get tagged file.
	 * @return tagged file.
	 */
	TaggedFile* getFile() { return m_file; }

	/**
	 * Set tagged file.
	 * The item takes ownership of this file and the old file is deleted.
	 *
	 * @param file tagged file.
	 */
	void setFile(TaggedFile* file);
 
	/**
	 * Mark file as selected.
	 *
	 * @param val true to set file selected.
	 */
	void setInSelection(bool val) { m_selected = val; }

	/**
	 * Check if file is marked selected.
	 *
	 * @return true if file is marked selected.
	 */
	bool isInSelection(void) { return m_selected; }

	/**
	 * Get height of item.
	 *
	 * @param lb listbox containing the item
	 *
	 * @return height.
	 */
	int height(const QListBox* lb) const;

	/**
	 * Get width of item.
	 *
	 * @param lb listbox containing the item
	 *
	 * @return width.
	 */
	int width(const QListBox* lb) const;

protected:
	/**
	 * Paint item.
	 *
	 * @param painter painter used
	 */
	void paint(QPainter *painter);

private:
	FileListItem(const FileListItem&);
	FileListItem& operator=(const FileListItem&);

	/** the tagged file represented by this item */
	TaggedFile* m_file;

	/** true if file is marked selected */
	bool m_selected;

	/** pointer to pixmap for modified file */
	static QPixmap *modifiedPixmap;
	/** pointer to empty pixmap */
	static QPixmap *nullPixmap;
	/** pointer to V1V2 pixmap */
	static QPixmap *v1v2Pixmap;
	/** pointer to V1 pixmap */
	static QPixmap *v1Pixmap;
	/** pointer to V2 pixmap */
	static QPixmap *v2Pixmap;
	/** pointer to "no tag" pixmap */
	static QPixmap *notagPixmap;
};

#endif // FILELISTITEM_H
