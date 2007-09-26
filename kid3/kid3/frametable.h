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

#if QT_VERSION < 0x040000
	/**
	 * @return preferred size.
	 */
	virtual QSize sizeHint() const;
#endif

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
	 * @return filter with enabled frames.
	 */
	FrameFilter getEnabledFrameFilter() const;

	/**
	 * Get reference to frame collection.
	 * @return frame collection.
	 */
	FrameCollection& frames() { return m_frames; }

public slots:
	/**
	 * Called when a value in the table is changed.
	 *
	 * @param row table row of changed item
	 * @param col table column of changed item
	 */
	void valueChanged(int row, int col);

	/**
	 * Called to trigger resizing in the next call to framesToTable().
	 */
	void triggerResize();

protected:
	/**
	 * Receive key press events.
	 * Reimplemented to use the Return key to start editing a cell, this can
	 * also be done with F2, but Return seems to be more intuitive.
	 *
	 * @param event event
	 */
	virtual void keyPressEvent(QKeyEvent* event);

#if QT_VERSION < 0x040000
	/**
	 * Called when a cell is painted.
	 * Paint the first cell of marked rows with red background.
	 * @param p painter
	 * @param row column
	 * @param col column
	 * @param cr  cell rectangle
	 * @param selected true if selected
	 * @param cg color group
	 */
	virtual void paintCell(QPainter* p, int row, int col, const QRect& cr,
												 bool selected, const QColorGroup& cg);
#endif

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
#if QT_VERSION < 0x040000
	bool m_resizeTable;
#endif
	FrameCollection m_frames;
};

#endif // FRAMETABLE_H
