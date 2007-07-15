/**
 * \file stringlistedit.h
 * Widget to edit a string list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Apr 2007
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
