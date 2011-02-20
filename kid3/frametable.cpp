/**
 * \file frametable.cpp
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

#include "frametable.h"
#include "kid3.h"
#include "genres.h"
#include <QComboBox>
#include <QAction>
#include <QPoint>
#include <QHeaderView>
#include <QKeyEvent>
#include <QItemDelegate>
#include <QApplication>
#include <QMenu>

/** Column indices. */
enum ColumnIndex {
	CI_Enable,
	CI_Value,
	CI_NumColumns
};

/**
 * Constructor.
 * @param parent parent widget
 */
FrameTableLineEdit::FrameTableLineEdit(QWidget* parent) :
	QLineEdit(parent)
{
	connect(this, SIGNAL(textChanged(const QString&)),
					this, SLOT(formatTextIfEnabled(const QString&)));
}

/**
 * Destructor.
 */
FrameTableLineEdit::~FrameTableLineEdit() {}

/**
 * Format text if enabled.
 * @param txt text to format and set in line edit
 */
void FrameTableLineEdit::formatTextIfEnabled(const QString& txt)
{
	if (Kid3App::s_id3FormatCfg.m_formatWhileEditing) {
		QString str(txt);
		Kid3App::s_id3FormatCfg.formatString(str);
		if (str != txt) {
			int curPos = cursorPosition();
			setText(str);
			setCursorPosition(curPos);
		}
	}
}

/** Delegate for table widget items. */
class FrameItemDelegate : public QItemDelegate {
public:
	enum {
		RttiValueGenre = 0x6e21e, /**< RTTI value for genre items */
		RttiValue28,
		RttiValue30
	};

	/**
	 * Constructor.
	 * @param parent parent QTableWidget
	 */
	FrameItemDelegate(QObject* parent) : QItemDelegate(parent) {}

	/**
	 * Destructor.
	 */
	virtual ~FrameItemDelegate() {}

	/**
	 * Create an editor to edit the cells contents.
	 * @param parent parent widget
	 * @param option style
	 * @param index  index of item
	 * @return combo box editor widget.
	 */
	virtual QWidget* createEditor(
		QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	/**
	 * Set data to be edited by the editor.
	 * @param editor editor widget
	 * @param index  index of item
	 */
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;

	/**
	 * Set model data supplied by editor.
	 * @param editor editor widget
	 * @param model  model
	 * @param index  index of item
	 */
	virtual void setModelData(
		QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
};
 
/**
 * Create an editor to edit the cells contents.
 * @param parent parent widget
 * @param option style
 * @param index  index of item
 * @return combo box editor widget.
 */
QWidget* FrameItemDelegate::createEditor(
	QWidget* parent, const QStyleOptionViewItem& option,
	const QModelIndex& index) const
{
	FrameTable* ft = qobject_cast<FrameTable*>(this->parent());
	int row = index.row();
	int col = index.column();
	if (col == CI_Value && row >= 0 && ft) {
		int type = ft->item(row, col)->type();
		if (type == RttiValueGenre) {
			QComboBox* cb = new QComboBox(parent);
			if (cb) {
				bool id3v1 = ft->isId3v1();
				if (!id3v1) {
					cb->setEditable(true);
					cb->setAutoCompletion(true);
					cb->setDuplicatesEnabled(false);
				}

				QStringList strList;
				for (const char** sl = Genres::s_strList; *sl != 0; ++sl) {
					strList += *sl;
				}
				if (Kid3App::s_miscCfg.m_onlyCustomGenres) {
					cb->addItem("");
				} else {
					cb->addItems(strList);
				}
				if (id3v1) {
					for (QStringList::const_iterator it =
								 Kid3App::s_miscCfg.m_customGenres.begin();
							 it != Kid3App::s_miscCfg.m_customGenres.end();
							 ++it) {
						if (Genres::getNumber(*it) != 255) {
							cb->addItem(*it);
						}
					}
				} else {
					cb->addItems(Kid3App::s_miscCfg.m_customGenres);
				}
			}
			return cb;
		} else if (type == RttiValue28 || type == RttiValue30) {
			FrameTableLineEdit* e = new FrameTableLineEdit(parent);
			e->setMaxLength(type == RttiValue28 ? 28 : 30);
			e->setFrame(false);
			return e;
		}
	}
	return QItemDelegate::createEditor(parent, option, index);
}
 
/**
 * Set data to be edited by the editor.
 * @param editor editor widget
 * @param index  index of item
 */
void FrameItemDelegate::setEditorData(
	QWidget* editor, const QModelIndex& index) const
{
	QComboBox* cb = qobject_cast<QComboBox*>(editor);
	if (cb) {
		QString genreStr(index.model()->data(index).toString());
		int genreIndex = genreStr.isNull() ? 0 :
			Genres::getIndex(Genres::getNumber(genreStr));
		if (Kid3App::s_miscCfg.m_onlyCustomGenres) {
			genreIndex = cb->findText(genreStr);
			if (genreIndex < 0) genreIndex = 0;
		} else if (genreIndex <= 0) {
			genreIndex = cb->findText(genreStr);
			if (genreIndex < 0) genreIndex = Genres::count + 1;
		}
		cb->setItemText(genreIndex, genreStr);
		cb->setCurrentIndex(genreIndex);
	} else {
		QItemDelegate::setEditorData(editor, index);
	}
}
 
/**
 * Set model data supplied by editor.
 * @param editor editor widget
 * @param model  model
 * @param index  index of item
 */
void FrameItemDelegate::setModelData(
	QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
	QComboBox* cb = qobject_cast<QComboBox *>(editor);
	if (cb) {
		model->setData(index, cb->currentText());
	} else {
		QItemDelegate::setModelData(editor, model, index);
	}
}


/**
 * Constructor.
 *
 * @param parent parent widget
 * @param id3v1  true if table for ID3v1 frames
 */
FrameTable::FrameTable(QWidget* parent, bool id3v1) :
	QTableWidget(parent), m_cursorRow(-1), m_cursorColumn(-1),
	m_markedRows(0), m_changedFrames(0), m_setCheckBoxes(true), m_id3v1(id3v1),
	m_currentEditor(0)
{
	setColumnCount(CI_NumColumns);
	setSelectionMode(SingleSelection);
	horizontalHeader()->setResizeMode(CI_Value, QHeaderView::Stretch);
	horizontalHeader()->hide();
	verticalHeader()->hide();
	setRowCount(1);
	if (id3v1) {
		setMinimumHeight((Frame::FT_LastV1Frame + 1) * (rowHeight(0) + 1));
	}
	QTableWidgetItem* twi = new QTableWidgetItem(i18n("Track Number") + "WW");
	twi->setCheckState(Qt::Unchecked);
	setItem(0, CI_Enable, twi);
	resizeColumnToContents(CI_Enable);
	horizontalHeader()->setResizeMode(CI_Value, QHeaderView::Stretch);
	removeRow(0);
	setItemDelegate(new FrameItemDelegate(this));
	setEditTriggers(AllEditTriggers);
	viewport()->installEventFilter(this); // keep track of editors
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(customContextMenu(const QPoint&)));
}

/**
 * Destructor.
 */
FrameTable::~FrameTable() {}

/**
 * Get a display representation of the a frame name.
 * For ID3v2-IDs with description, only the ID is returned.
 * Other non-empty strings are translated.
 *
 * @param str frame name
 *
 * @return display representation of name.
 */
QString FrameTable::getDisplayName(const QString& str) const
{
	if (!str.isEmpty()) {
		int nlPos = str.indexOf("\n");
		if (nlPos > 0) {
			// probably "TXXX - User defined text information\nDescription" or
			// "WXXX - User defined URL link\nDescription"
			return str.mid(nlPos + 1);
		} else if (str.mid(4, 3) == " - ") {
			// probably "ID3-ID - Description"
			return str.left(4);
		} else {
			return QCM_translate(str.toLatin1().data());
		}
	}
	return str;
}

/**
 * Display frames in table.
 */
void FrameTable::framesToTable()
{
	setRowCount(m_frames.size());
	int row = 0;
	for (FrameCollection::const_iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		QTableWidgetItem* twi;
		if ((twi = item(row, CI_Enable)) != 0) {
			twi->setText(getDisplayName((*it).getName()));
		} else {
			twi = new QTableWidgetItem(getDisplayName((*it).getName()));
			twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
			setItem(row, CI_Enable, twi);
		}
		twi->setCheckState(m_setCheckBoxes ? Qt::Checked : Qt::Unchecked);
		bool frameChanged = 
			(static_cast<unsigned>((*it).getType()) < sizeof(m_changedFrames) * 8 &&
			 (m_changedFrames & (1 << (*it).getType())) != 0);
		twi->setBackground(frameChanged ? QApplication::palette().mid() : Qt::NoBrush);

		int type;
		if (m_id3v1) {
			switch ((*it).getType()) {
				case Frame::FT_Genre:
					type = (int)FrameItemDelegate::RttiValueGenre;
					break;
				case Frame::FT_Comment:
					type = (int)FrameItemDelegate::RttiValue28;
					break;
				case Frame::FT_Title:
				case Frame::FT_Artist:
				case Frame::FT_Album:
					type = (int)FrameItemDelegate::RttiValue30;
					break;
				default:
					type = (int)QTableWidgetItem::Type;
			}
		} else {
			type = ((*it).getType() != Frame::FT_Genre) ?
				(int)QTableWidgetItem::Type : (int)FrameItemDelegate::RttiValueGenre;
		}
		if ((twi = item(row, CI_Value)) != 0 &&
				twi->type() == type) {
			twi->setText((*it).getValue());
		} else {
			twi = new QTableWidgetItem((*it).getValue(), type);
			setItem(row, CI_Value, twi);
		}
		if (row < 8 && twi) {
			twi->setBackground((m_markedRows & (1 << row)) != 0 ?
												 QBrush(Qt::red) : Qt::NoBrush);
		}

		++row;
	}
	clearSelection();
}

/**
 * Set frames from values in table.
 *
 * @param setUnchanged if true, also set marked values which are unchanged,
 *                     which can be used if multiple files are selected
 */
void FrameTable::tableToFrames(bool setUnchanged)
{
	acceptEdit(); // commit edits from open editors in the table
	int row = 0;
	for (FrameCollection::iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		QString value;
		QTableWidgetItem* twi;
		if ((twi = item(row, CI_Enable)) != 0 &&
				twi->checkState() == Qt::Checked &&
				(twi = item(row, CI_Value)) != 0 &&
				((value = twi->text()) != (*it).getValue() || setUnchanged)) {
			Frame& frame = const_cast<Frame&>(*it);
			if (value.isNull()) value = "";
			frame.setValueIfChanged(value);
		}
		++row;
	}
}

/**
 * Save the current cursor position.
 */
void FrameTable::saveCursor()
{
	m_cursorRow = currentRow();
	m_cursorColumn = currentColumn();
}

/**
 * Restore the cursor position saved with saveCursor().
 */
void FrameTable::restoreCursor()
{
	int lastRow = rowCount() - 1;
	if (m_cursorRow >= 0 && m_cursorColumn >= 0 && lastRow >= 0) {
		if (m_cursorRow > lastRow) {
			m_cursorRow = lastRow;
		}
		setCurrentCell(m_cursorRow, m_cursorColumn);
	}
}

/**
 * Get current frame.
 * @return frame, 0 if no frame.
 */
const Frame* FrameTable::getCurrentFrame() const
{
	int row = currentRow();
	if (row < static_cast<int>(m_frames.size())) {
		int i = 0;
		for (FrameCollection::const_iterator it = m_frames.begin();
				 it != m_frames.end();
				 ++it) {
			if (i == row) {
				return &(*it);
			}
			++i;
		}
	}
	return 0;
}

/**
 * Select the row of the frame with a given index.
 *
 * @param index index of frame
 *
 * @return true if found.
 */
bool FrameTable::selectFrameWithIndex(int index)
{
	int row = 0;
	for (FrameCollection::const_iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		if (it->getIndex() == index) {
			if (row < rowCount()) {
				setCurrentCell(row, CI_Value);
			}
			return true;
		}
		++row;
	}
	return false;
}

/**
 * Select the row of the frame with a given name.
 *
 * @param name name of frame
 *
 * @return true if found.
 */
bool FrameTable::selectFrameWithName(const QString& name)
{
	int row = 0;
	for (FrameCollection::const_iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		if (it->getName() == name) {
			if (row < rowCount()) {
				setCurrentCell(row, CI_Value);
			}
			return true;
		}
		++row;
	}
	return false;
}

/**
 * Get filter with enabled frames.
 *
 * @param allDisabledToAllEnabled true to enable all if all are disabled
 *
 * @return filter with enabled frames.
 */
FrameFilter FrameTable::getEnabledFrameFilter(
	bool allDisabledToAllEnabled) const
{
	FrameFilter filter;
	filter.enableAll();
	bool allDisabled = true;
	int numberRows = rowCount();
	QTableWidgetItem* ti;
	int row = 0;
	for (FrameCollection::const_iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		if (row >= numberRows) break;
		if ((ti = item(row, CI_Enable)) != 0 &&
				ti->checkState() == Qt::Unchecked) {
			filter.enable(it->getType(), it->getName(), false);
		} else {
			allDisabled = false;
		}
		++row;
	}
	if (allDisabledToAllEnabled && allDisabled) {
		filter.enableAll();
	}
	return filter;
}

/**
 * Filters events if this object has been installed as an event filter
 * for the watched object.
 * This method is reimplemented to keep track of the current open editor.
 * It has to be installed on the viewport of the table.
 * @param event event
 * @return true to filter event out.
 */
bool FrameTable::eventFilter(QObject*, QEvent* event)
{
	if (event) {
		QEvent::Type type = event->type();
		if (type == QEvent::ChildAdded) {
			QObject* obj = ((QChildEvent*)event)->child();
			if (obj && obj->isWidgetType()) {
				m_currentEditor = (QWidget*)obj;
			}
		} else if (type == QEvent::ChildRemoved) {
			if (m_currentEditor == ((QChildEvent*)event)->child()) {
				m_currentEditor = 0;
			}
		} else if (type == QEvent::WindowDeactivate) {
			// this is done to avoid losing focus when the window is deactivated
			// while editing a cell (i.e. the cell is not closed by pressing Enter)
			if ((state() == QAbstractItemView::EditingState) && m_currentEditor) {
				commitData(m_currentEditor);
				closeEditor(m_currentEditor, QAbstractItemDelegate::EditPreviousItem);
			}
		}
	}
	return false;
}

/**
 * Commit data from the current editor.
 * This is used to avoid loosing the changes in open editors e.g. when
 * the file is changed using Alt-Up or Alt-Down.
 *
 * @return true if data was committed.
 */
bool FrameTable::acceptEdit()
{
	if ((state() == QAbstractItemView::EditingState) && m_currentEditor) {
		commitData(m_currentEditor);
		//  close editor to avoid being stuck in QAbstractItemView::NoState
		closeEditor(m_currentEditor, QAbstractItemDelegate::NoHint);
		return true;
	}
	return false;
}


/**
 * Set the check state of all frames in the table.
 *
 * @param checked true to check the frames
 */
void FrameTable::setAllCheckStates(bool checked)
{
	for (int row = 0; row < rowCount(); ++row) {
		QTableWidgetItem* twi = item(row, CI_Enable);
		if (twi) {
			twi->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
		}
	}
}

/**
 * Select all frames in the table.
 */
void FrameTable::selectAllFrames()
{
	setAllCheckStates(true);
}

/**
 * Deselect all frames in the table.
 */
void FrameTable::deselectAllFrames()
{
	setAllCheckStates(false);
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
void FrameTable::executeAction(QAction* action)
{
	if (action) {
		int cmd = action->data().toInt();
		switch (cmd) {
			case 0:
				selectAllFrames();
				break;
			case 1:
			default:
				deselectAllFrames();
				break;
		}
	}
}

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void FrameTable::contextMenu(int row, int col, const QPoint& pos)
{
	if (col == 0 && row >= 0) {
		QMenu menu(this);
		QAction* action = menu.addAction(i18n("&Select all"));
		if (action) action->setData(0);
		action = menu.addAction(i18n("&Deselect all"));
		if (action) action->setData(1);
		connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(executeAction(QAction*)));
		menu.setMouseTracking(true);
		menu.exec(pos);
	}
}

/**
 * Display custom context menu.
 *
 * @param pos position where context menu is drawn on screen
 */
void FrameTable::customContextMenu(const QPoint& pos)
{
	QTableWidgetItem* item = itemAt(pos);
	if (item) {
		contextMenu(item->row(), item->column(), mapToGlobal(pos));
	}
}
