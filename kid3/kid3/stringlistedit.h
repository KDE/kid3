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

#include <qwidget.h>
#include "qtcompatmac.h"

#if QT_VERSION >= 0x040000
class QListWidget;
#else
class QListBox;
#endif
class QPushButton;

/**
 * Widget to edit a string list.
 */
class StringListEdit : public QWidget {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	StringListEdit(QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	~StringListEdit();

	/**
	 * Set the string list in the list box.
	 *
	 * @param strList string list
	 */
	void setStrings(const QStringList& strList);

	/**
	 * Store the string list from the list box.
	 *
	 * @param strList the string list is stored here
	 */
	void getStrings(QStringList& strList) const;

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
#if QT_VERSION >= 0x040000
	QListWidget* m_stringListBox;
#else
	QListBox* m_stringListBox;
#endif
	QPushButton* m_addPushButton;
	QPushButton* m_moveUpPushButton;
	QPushButton* m_moveDownPushButton;
	QPushButton* m_editPushButton;
	QPushButton* m_removePushButton;
};

#endif // STRINGLISTEDIT_H
