/**
 * \file frametable.h
 * Table to edit frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 05 Sep 2007
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

#ifndef FRAMETABLE_H
#define FRAMETABLE_H

#include "frame.h"
#include <qstringlist.h>
#include <qlineedit.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QTableWidget>
typedef QTableWidget FrameTableBaseClass;
#else 
#include <qtable.h>
typedef QTable FrameTableBaseClass;
#endif

/**
 * Table to edit frames.
 */
class FrameTable : public FrameTableBaseClass {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 * @param id3v1  true if table for ID3v1 frames
	 */
	explicit FrameTable(QWidget* parent = 0, bool id3v1 = false);

	/**
	 * Destructor.
	 */
	virtual ~FrameTable();

	/**
	 * Check if table is for ID3v1 frames.
	 * @return true if for ID3v1.
	 */
	bool isId3v1() const { return m_id3v1; }

	/**
	 * Mark rows.
	 * @param rowMask mask with bits of rows to mark
	 */
	void markRows(unsigned char rowMask) { m_markedRows = rowMask; }

	/**
	 * Set all check boxes on or off.
	 * Will take effect when framesToTable() is called.
	 *
	 * @param val true to set check boxes on.
	 */
	void setAllCheckBoxes(bool val) { m_setCheckBoxes = val; }

	/**
	 * Display frames in table.
	 */
	void framesToTable();

	/**
	 * Set frames from values in table.
	 */
	void tableToFrames();

	/**
	 * Save the current cursor position.
	 */
	void saveCursor();

	/**
	 * Restore the cursor position saved with saveCursor().
	 */
	void restoreCursor();

	/**
	 * Get current frame.
	 * @return frame, 0 if no frame.
	 */
	const Frame* getCurrentFrame() const;

	/**
	 * Select the row of the frame with a given index.
	 *
	 * @param index index of frame
	 *
	 * @return true if found.
	 */
	bool selectFrameWithIndex(int index);

	/**
	 * Select the row of the frame with a given name.
	 *
	 * @param index name of frame
	 *
	 * @return true if found.
	 */
	bool selectFrameWithName(const QString& name);

	/**
	 * Get filter with enabled frames.
	 *
	 * @param allDisabledToAllEnabled true to enable all if all are disabled
	 *
	 * @return filter with enabled frames.
	 */
	FrameFilter getEnabledFrameFilter(bool allDisabledToAllEnabled = false) const;

	/**
	 * Get reference to frame collection.
	 * @return frame collection.
	 */
	FrameCollection& frames() { return m_frames; }

public slots:
	/**
	 * Called to trigger resizing in the next call to framesToTable().
	 */
	void triggerResize();

private:
	/**
	 * Get a display representation of the a frame name.
	 * For ID3v2-IDs with description, only the ID is returned.
	 * Other non-empty strings are translated.
	 *
	 * @param str frame name
	 *
	 * @return display representation of name.
	 */
	QString getDisplayName(const QString& str) const;

	int m_cursorRow;
	int m_cursorColumn;
	unsigned char m_markedRows;
	bool m_setCheckBoxes;
	const bool m_id3v1;
	FrameCollection m_frames;

#if QT_VERSION < 0x040000
public:
	/**
	 * @return preferred size.
	 */
	virtual QSize sizeHint() const;

	/**
	 * Filters events if this object has been installed as an event filter
	 * for the watched object.
	 * @param o watched object
	 * @param e event
	 * @return true to filter event out.
	 */
	virtual bool eventFilter(QObject* o, QEvent* e);

private:
	bool m_resizeTable;
#endif
};

/** Line edit with automatic tag formatting. */
class FrameTableLineEdit : public QLineEdit {
Q_OBJECT
public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 */
	FrameTableLineEdit(QWidget* parent);

	/**
	 * Destructor.
	 */
	virtual ~FrameTableLineEdit();

#if QT_VERSION < 0x040000
	/**
	 * Set the table item using this line edit.
	 * @param ti table item
	 */
	void setTableItem(const QTableItem* ti) { m_tableItem = ti; }

protected:
	/**
	 * Called when the widget gets the keyboard focus.
	 * Used to set the current table cell, because this is not done
	 * when using EditType Always.
	 * @param event focus event
	 */
	virtual void focusInEvent(QFocusEvent* event);

private:
	const QTableItem* m_tableItem;
#endif

private slots:
	/**
	 * Format text if enabled.
	 * @param txt text to format and set in line edit
	 */
	void formatTextIfEnabled(const QString& txt);
};

#endif // FRAMETABLE_H
