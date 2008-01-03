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
#include <qcombobox.h>
#if QT_VERSION >= 0x040000
#include <QHeaderView>
#include <QKeyEvent>
#include <QItemDelegate>
#endif

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
#if QT_VERSION < 0x040000
, m_tableItem(0)
#endif
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

#if QT_VERSION < 0x040000
/**
 * Called when the widget gets the keyboard focus.
 * Used to set the current table cell, because this is not done
 * when using EditType Always.
 * @param event focus event
 */
void FrameTableLineEdit::focusInEvent(QFocusEvent* event)
{
	if (m_tableItem) {
		QTable* table = m_tableItem->table();
		if (table) {
			table->setCurrentCell(m_tableItem->row(), m_tableItem->col());
		}
	}
	QLineEdit::focusInEvent(event);
}

class FrameTableComboBox : public QComboBox {
public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 */
	FrameTableComboBox(QWidget* parent);

	/**
	 * Destructor.
	 */
	virtual ~FrameTableComboBox();

	/**
	 * Set the table item using this line edit.
	 * @param ti table item
	 */
	void setTableItem(const QTableItem* ti) { m_tableItem = ti; }

protected:
	/**
	 * Called when the widget is clicked by the mouse.
	 * Used to set the current table cell, because this is not done
	 * when using EditType Always.
	 * @param event mouse event
	 */
	virtual void mousePressEvent(QMouseEvent* event);

private:
	const QTableItem* m_tableItem;
};

/**
 * Constructor.
 * @param parent parent widget
 */
FrameTableComboBox::FrameTableComboBox(QWidget* parent) :
	QComboBox(parent), m_tableItem(0) {}

/**
 * Destructor.
 */
FrameTableComboBox::~FrameTableComboBox() {}

/**
 * Called when the widget is clicked by the mouse.
 * Used to set the current table cell, because this is not done
 * when using EditType Always.
 * @param event mouse event
 */
void FrameTableComboBox::mousePressEvent(QMouseEvent* event)
{
	if (m_tableItem) {
		QTable* table = m_tableItem->table();
		if (table) {
			table->setCurrentCell(m_tableItem->row(), m_tableItem->col());
		}
	}
	QComboBox::mousePressEvent(event);
}
#endif


#if QT_VERSION >= 0x040000
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
 
#else

/** QTableItem subclass to align everything (also numbers) left. */
class ValueTableItem : public QTableItem {
public:
	enum {
		RttiValue = 0xba13e, /**< RTTI value for value items */
		RttiValue28,
		RttiValue30
	};

	/**
	 * Constructor.
	 * @param table table
	 * @param text  text
	 * @param type  type (one of the RttiValue enum)
	 */
	ValueTableItem(QTable* table, const QString& text, int type);

	/**
	 * Destructor.
	 */
	virtual ~ValueTableItem();

	/**
	 * Create an editor to edit the cells contents.
	 * @return combo box editor widget.
	 */
	virtual QWidget* createEditor() const;

	/** Alignment. @return AlignLeft. */
	virtual int alignment() const { return AlignLeft; }

	/**
	 * Set text of table item.
	 * @param t text
	 */
	virtual void setText(const QString& t);

	/**
	 * Get text of table item.
	 * @return text.
	 */
	virtual QString text() const;

	/**
	 * Get runtime type identification.
	 * @return RttiValue.
	 */
	virtual int rtti() const { return m_type; }

	/**
	 * Set red color.
	 * @param en true to enable
	 */
	void setColorRed(bool en);

private:
	const int m_type;
};

/**
 * Constructor.
 * @param table table
 * @param text  text
 * @param type  type (one of the RttiValue enum)
 */
ValueTableItem::ValueTableItem(QTable* table, const QString& text, int type) :
	QTableItem(table, Always, text), m_type(type)
{
	setReplaceable(false);
}

/**
 * Destructor.
 */
ValueTableItem::~ValueTableItem() {}

/**
 * Create an editor to edit the cells contents.
 * @return line edit widget.
 */
QWidget* ValueTableItem::createEditor() const
{
	FrameTableLineEdit* e = new FrameTableLineEdit(table()->viewport());
	if (m_type == RttiValue28) {
		e->setMaxLength(28);
	} else if (m_type == RttiValue30) {
		e->setMaxLength(30);
	}
	e->setFrame(false);
	e->setText(text());
	e->setTableItem(this);
	return e;
}

/**
 * Set text of table item.
 * @param t text
 */
void ValueTableItem::setText(const QString& t)
{
	QTableItem::setText(t);
	QWidget* w = table()->cellWidget(row(), col());
	QLineEdit* le = ::qt_cast<QLineEdit*>(w);
	if (le) {
		le->setText(t);
	}
}

/**
 * Get text of table item.
 * @return text.
 */
QString ValueTableItem::text() const
{
	QWidget* w = table()->cellWidget(row(), col());
	QLineEdit* le = ::qt_cast<QLineEdit*>(w);
	if (le) {
		return le->text();
	} else {
		return QTableItem::text();
	}
}

/**
 * Set red color.
 * @param en true to enable
 */
void ValueTableItem::setColorRed(bool en)
{
	QWidget* w = table()->cellWidget(row(), col());
	QLineEdit* le = ::qt_cast<QLineEdit*>(w);
	if (le) {
		if (en) {
			le->setPaletteBackgroundColor(Qt::red);
		} else {
			le->unsetPalette();
		}
	}
}


/** QTableItem with combo box to edit genres. */
class GenreTableItem : public QTableItem {
public:
	enum {
		RttiValue = 0x6e21e /**< RTTI value for genre items */
	};

	/**
	 * Constructor.
	 * @param table table
	 * @param text  text
	 */
	GenreTableItem(QTable* table, const QString& text);

	/**
	 * Destructor.
	 */
	virtual ~GenreTableItem();

	/**
	 * Create an editor to edit the cells contents.
	 * @return combo box editor widget.
	 */
	virtual QWidget* createEditor() const;

	/**
	 * Copy content from editor into QTableItem.
	 * @param w editor widget
	 */
	virtual void setContentFromEditor(QWidget *w);

	/**
	 * Set text of table item.
	 * @param t text
	 */
	virtual void setText(const QString& t);

	/**
	 * Get text of table item.
	 * @return text.
	 */
	virtual QString text() const;

	/**
	 * Get runtime type identification.
	 * @return RttiValue.
	 */
	virtual int rtti() const { return RttiValue; }

	/**
	 * Set red color.
	 * @param en true to enable
	 */
	void setColorRed(bool en);
};

/**
 * Constructor.
 * @param table table
 * @param text  text
 */
GenreTableItem::GenreTableItem(QTable* table, const QString& text) :
	QTableItem(table, Always, text)
{
	setReplaceable(false);
}

/**
 * Destructor.
 */
GenreTableItem::~GenreTableItem() {}

/**
 * Create an editor to edit the cells contents.
 * @return combo box editor widget.
 */
QWidget* GenreTableItem::createEditor() const
{
	FrameTableComboBox* cb = new FrameTableComboBox(table()->viewport());
	if (cb) {
		cb->setTableItem(this);
		FrameTable* ft = dynamic_cast<FrameTable*>(table());
		bool id3v1 = ft && ft->isId3v1();
		QObject::connect(cb, SIGNAL(activated(int)),
										 table(), SLOT(doValueChanged()));
		if (!id3v1) {
			cb->setEditable(true);
			FrameTableLineEdit* ftle = new FrameTableLineEdit(cb);
			if (ftle) {
				ftle->setTableItem(this);
				cb->setLineEdit(ftle);
			}
			cb->setAutoCompletion(true);
			cb->setDuplicatesEnabled(false);
			QLineEdit* le = cb->lineEdit();
			if (le) {
				le->installEventFilter(table());
			}
		}

		if (Kid3App::s_miscCfg.m_onlyCustomGenres) {
			cb->insertItem("");
		} else {
			cb->insertStrList(Genres::s_strList);
		}
		if (id3v1) {
			for (QStringList::const_iterator it =
						 Kid3App::s_miscCfg.m_customGenres.begin();
					 it != Kid3App::s_miscCfg.m_customGenres.end();
					 ++it) {
				if (Genres::getNumber(*it) != 255) {
					cb->insertItem(*it);
				}
			}
		} else {
			cb->insertStringList(Kid3App::s_miscCfg.m_customGenres);
		}

		QString genreStr = text();
		int genreIndex = genreStr.isNull() ? 0 :
			Genres::getIndex(Genres::getNumber(genreStr));
		cb->setCurrentItem(
			Kid3App::s_miscCfg.m_onlyCustomGenres ? 0 :
			(genreIndex > 0 ? genreIndex : Genres::count + 1));
		cb->setCurrentText(genreStr);
	}
	return cb;
}

/**
 * Copy content from editor into QTableItem.
 * @param w editor widget
 */
void GenreTableItem::setContentFromEditor(QWidget *w)
{
	if (w->inherits("QComboBox")) {
		setText(((QComboBox*)w)->currentText());
	} else {
		QTableItem::setContentFromEditor(w);
	}
}

/**
 * Set text of table item.
 * @param t text
 */
void GenreTableItem::setText(const QString& t)
{
	QTableItem::setText(t);
	QWidget* w = table()->cellWidget(row(), col());
	QComboBox* cb = ::qt_cast<QComboBox*>(w);
	if (cb) {
		int genreIndex = t.isNull() ? 0 :
			Genres::getIndex(Genres::getNumber(t));
		cb->setCurrentItem(
			Kid3App::s_miscCfg.m_onlyCustomGenres ? 0 :
			(genreIndex > 0 ? genreIndex : Genres::count + 1));
		cb->setCurrentText(t);
	}
}

/**
 * Get text of table item.
 * @return text.
 */
QString GenreTableItem::text() const
{
	QWidget* w = table()->cellWidget(row(), col());
	QComboBox* cb = ::qt_cast<QComboBox*>(w);
	if (cb) {
		return cb->currentText();
	} else {
		return QTableItem::text();
	}
}

/**
 * Set red color.
 * @param en true to enable
 */
void GenreTableItem::setColorRed(bool en)
{
	QWidget* w = table()->cellWidget(row(), col());
	QComboBox* cb = ::qt_cast<QComboBox*>(w);
	if (cb) {
		if (en) {
			QPalette p = cb->palette();
#ifdef WIN32
			p.setColor(QColorGroup::Base, Qt::red);
#else
			p.setColor(QColorGroup::Button, Qt::red);
#endif
			cb->setPalette(p);
		} else {
			cb->unsetPalette();
		}
	}
}

#endif


/**
 * Constructor.
 *
 * @param parent parent widget
 * @param id3v1  true if table for ID3v1 frames
 */
#if QT_VERSION >= 0x040000
FrameTable::FrameTable(QWidget* parent, bool id3v1) :
	QTableWidget(parent), m_cursorRow(-1), m_cursorColumn(-1),
	m_markedRows(0), m_setCheckBoxes(true), m_id3v1(id3v1)
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
	QTableWidgetItem* twi = new QTableWidgetItem(i18n("Track Number"));
	twi->setCheckState(Qt::Unchecked);
	setItem(0, CI_Enable, twi);
	resizeColumnToContents(CI_Enable);
	horizontalHeader()->setResizeMode(CI_Value, QHeaderView::Stretch);
	removeRow(0);
	setItemDelegate(new FrameItemDelegate(this));
	setEditTriggers(AllEditTriggers);
}
#else
FrameTable::FrameTable(QWidget* parent, bool id3v1) :
	QTable(parent), m_cursorRow(-1), m_cursorColumn(-1),
	m_markedRows(0), m_id3v1(id3v1), m_resizeTable(false), m_updateGenres(false)
{
	setNumCols(CI_NumColumns);
	setSelectionMode(NoSelection);
	horizontalHeader()->hide();
	setTopMargin(0);
	verticalHeader()->hide();
	setLeftMargin(0);
	setNumRows(1);
	if (id3v1) {
		setMinimumHeight((Frame::FT_LastV1Frame + 1) * (rowHeight(0) + 1));
	}
	setItem(0, CI_Enable, new QCheckTableItem(this, i18n("Track Number")));
	adjustColumn(CI_Enable);
	setColumnStretchable(CI_Value, true);
	removeRow(0);
}
#endif

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
		if (str.mid(4, 3) == " - ") {
			// probably "ID3-ID - Description"
			return str.left(4);
		} else {
#if QT_VERSION >= 0x040000
			return QCM_translate(str.toLatin1().data());
#else
			return QCM_translate(str);
#endif
		}
	}
	return str;
}

/**
 * Display frames in table.
 */
void FrameTable::framesToTable()
{
#if QT_VERSION >= 0x040000
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
#if QT_VERSION >= 0x040200
			twi->setBackground((m_markedRows & (1 << row)) != 0 ?
												 QBrush(Qt::red) : Qt::NoBrush);
#elif QT_VERSION >= 0x040000
			twi->setBackgroundColor((m_markedRows & (1 << row)) != 0 ?
															Qt::red : QColor());
#endif
		}

		++row;
	}
	clearSelection();
#else
	if (m_resizeTable) {
		resize(minimumSize());
		m_resizeTable = false;
	}
	setNumRows(m_frames.size());
	int row = 0;
	QTableItem* ti;
	QCheckTableItem* cti;
	for (FrameCollection::const_iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		if ((ti = item(row, CI_Enable)) != 0 &&
				(cti = dynamic_cast<QCheckTableItem*>(ti)) != 0) {
			ti->setText(getDisplayName((*it).getName()));
		} else {
			cti = new QCheckTableItem(this, getDisplayName((*it).getName()));
			setItem(row, CI_Enable, cti);
		}
		if (cti) {
			cti->setChecked(m_setCheckBoxes);
		}

		int type;
		if (m_id3v1) {
			switch ((*it).getType()) {
				case Frame::FT_Genre:
					type = (int)GenreTableItem::RttiValue;
					break;
				case Frame::FT_Comment:
					type = (int)ValueTableItem::RttiValue28;
					break;
				case Frame::FT_Title:
				case Frame::FT_Artist:
				case Frame::FT_Album:
					type = (int)ValueTableItem::RttiValue30;
					break;
				default:
					type = (int)ValueTableItem::RttiValue;
			}
		} else {
			type = ((*it).getType() != Frame::FT_Genre) ?
				(int)ValueTableItem::RttiValue : (int)GenreTableItem::RttiValue;
		}

		if (!(m_updateGenres && (*it).getType() == Frame::FT_Genre) &&
				(ti = item(row, CI_Value)) != 0 &&
				ti->rtti() == type) {
			ti->setText((*it).getValue());
		} else {
			if ((*it).getType() != Frame::FT_Genre) {
				ti = new ValueTableItem(this, (*it).getValue(), type);
			} else {
				ti = new GenreTableItem(this, (*it).getValue());
			}
			setItem(row, CI_Value, ti);

			if (m_updateGenres && (*it).getType() == Frame::FT_Genre) {
				m_updateGenres = false;
			}
		}
		if (row < 8 && ti) {
			ValueTableItem* vti;
			GenreTableItem* gti;
			if ((vti = dynamic_cast<ValueTableItem*>(ti)) != 0) {
				vti->setColorRed((m_markedRows & (1 << row)) != 0);
			} else if ((gti = dynamic_cast<GenreTableItem*>(ti)) != 0) {
				gti->setColorRed((m_markedRows & (1 << row)) != 0);
			}
		}

		updateContents();

		++row;
	}
#endif
}

/**
 * Set frames from values in table.
 */
void FrameTable::tableToFrames()
{
#if QT_VERSION >= 0x040000
	int row = 0;
	for (FrameCollection::iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		QString value;
		QTableWidgetItem* twi;
		if ((twi = item(row, CI_Enable)) != 0 &&
				twi->checkState() == Qt::Checked &&
				(twi = item(row, CI_Value)) != 0 &&
				(value = twi->text()) != (*it).getValue()) {
			Frame& frame = const_cast<Frame&>(*it);
			frame.setValue(value);
			frame.setValueChanged();
		}
		++row;
	}
#else
	int row = 0;
	for (FrameCollection::iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		QString value;
		QTableItem* ti;
		QCheckTableItem* cti;
		QComboTableItem* comboItem;
		if ((ti = item(row, CI_Enable)) != 0 &&
				(cti = dynamic_cast<QCheckTableItem*>(ti)) != 0 &&
				cti->isChecked()) {
			ti = item(row, CI_Value);
			if (ti && ti->rtti() == 1 &&
					(comboItem = dynamic_cast<QComboTableItem*>(ti)) != 0) {
				value = comboItem->currentText();
			} else {
				value = text(row, CI_Value);
			}
			if (value != (*it).getValue() &&
					!(value.isEmpty() && (*it).getValue().isEmpty())) {
				Frame& frame = const_cast<Frame&>(*it);
				frame.setValue(value);
				frame.setValueChanged();
			}
		}
		++row;
	}
#endif
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
#if QT_VERSION >= 0x040000
	int lastRow = rowCount() - 1;
#else
	int lastRow = numRows() - 1;
#endif
	if (m_cursorRow >= 0 && m_cursorColumn >= 0 && lastRow >= 0) {
		if (m_cursorRow > lastRow) {
			m_cursorRow = lastRow;
		}
		setCurrentCell(m_cursorRow, m_cursorColumn);
#if QT_VERSION < 0x040000
		ensureCellVisible(m_cursorRow, m_cursorColumn);
#endif
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
			if (row <
#if QT_VERSION >= 0x040000
					rowCount()
#else
					numRows()
#endif
				) {
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
 * @param index name of frame
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
			if (row <
#if QT_VERSION >= 0x040000
					rowCount()
#else
					numRows()
#endif
				) {
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
#if QT_VERSION >= 0x040000
	int numberRows = rowCount();
	QTableWidgetItem* ti;
#else
	int numberRows = numRows();
	QTableItem* ti;
	QCheckTableItem* cti;
#endif
	int row = 0;
	for (FrameCollection::const_iterator it = m_frames.begin();
			 it != m_frames.end();
			 ++it) {
		if (row >= numberRows) break;
		if ((ti = item(row, CI_Enable)) != 0 &&
#if QT_VERSION >= 0x040000
				ti->checkState() == Qt::Unchecked
#else
				(cti = dynamic_cast<QCheckTableItem*>(ti)) != 0 &&
				!cti->isChecked()
#endif
			) {
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
 * Called to trigger resizing in the next call to framesToTable().
 */
void FrameTable::triggerResize()
{
#if QT_VERSION < 0x040000
	m_resizeTable = true;
#endif
}

#if QT_VERSION < 0x040000
/**
 * Filters events if this object has been installed as an event filter
 * for the watched object.
 * This method is reimplemented to avoid the filtering of the left
 * and right keys as done in QTable::eventFilter(), so that these
 * keys can be used to move the cursor inside the line edit controls.
 * @param o watched object
 * @param e event
 * @return true to filter event out.
 */
bool FrameTable::eventFilter(QObject* o, QEvent* e)
{
	if (e && e->type() == QEvent::KeyPress) {
		QKeyEvent* ke = (QKeyEvent*)e;
		if (ke->key() == Key_Left || ke->key() == Key_Right ||
				ke->key() == Key_Home || ke->key() == Key_End) {
			return false;
		} else if (ke->key() == Key_Tab || ke->key() == Key_BackTab) {
			int row = currentRow(), col = currentColumn();
			if (ke->key() == Key_Tab) {
				if (col >= numCols() - 1) {
					col = 0;
					if (row >= numRows() - 1) {
						row = 0;
					} else {
						++row;
					}
				} else {
					++col;
				}
			} else if (ke->key() == Key_BackTab) {
				if (col <= 0) {
					col = numCols() - 1;
					if (row <= 0) {
						row = numRows() - 1;
					} else {
						--row;
					}
				} else {
					--col;
				}
			}
			setCurrentCell(row, col);
			return true;
		}
	}
	return QTable::eventFilter(o, e);
}

/**
 * @return preferred size.
 */
QSize FrameTable::sizeHint() const
{
	return QScrollView::sizeHint();
}
#endif
