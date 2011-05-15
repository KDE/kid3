/**
 * \file stringlistedit.h
 * Widget to edit a string list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Apr 2007
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

#ifndef STRINGLISTEDIT_H
#define STRINGLISTEDIT_H

#include <QWidget>

class QListView;
class QPushButton;
class QAbstractItemModel;

/**
 * Widget to edit a string list.
 */
class StringListEdit : public QWidget {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param model item model, e.g. a QStringListModel
	 * @param parent parent widget
	 */
	explicit StringListEdit(QAbstractItemModel* model, QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	~StringListEdit();

public slots:
	/**
	 * Add a new item.
	 */
	void addItem();

	/**
	 * Remove the selected item.
	 */
	void removeItem();

	/**
	 * Edit the selected item.
	 */
	void editItem();

	/**
	 * Move the selected item up.
	 */
	void moveUpItem();

	/**
	 * Move the selected item down.
	 */
	void moveDownItem();

	/**
	 * Change state of buttons according to the current item and the count.
	 */
	void setButtonEnableState();

private:
	QListView* m_stringListBox;
	QPushButton* m_addPushButton;
	QPushButton* m_moveUpPushButton;
	QPushButton* m_moveDownPushButton;
	QPushButton* m_editPushButton;
	QPushButton* m_removePushButton;
};

#endif // STRINGLISTEDIT_H
