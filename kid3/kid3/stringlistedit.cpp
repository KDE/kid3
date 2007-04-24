/**
 * \file stringlistedit.cpp
 * Widget to edit a string list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Apr 2007
 */

#include "stringlistedit.h"
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qinputdialog.h>
#include <qlayout.h>
#if QT_VERSION >= 0x040000
#include <Q3ListBox>
#else
#include <qlistbox.h>
#endif

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   Qt object name
 */
StringListEdit::StringListEdit(QWidget* parent, const char* name) :
	QWidget(parent, name)
{
	QHBoxLayout* hlayout = new QHBoxLayout(this);
	m_stringListBox = new Q3ListBox(this);
	if (hlayout && m_stringListBox) {
		hlayout->setSpacing(6);
		hlayout->addWidget(m_stringListBox);
		QVBoxLayout* vlayout = new QVBoxLayout(hlayout);
		m_addPushButton = new QPushButton(i18n("&Add..."), this);
		m_moveUpPushButton = new QPushButton(i18n("Move &Up"), this);
		m_moveDownPushButton = new QPushButton(i18n("Move &Down"), this);
		m_editPushButton = new QPushButton(i18n("&Edit..."), this);
		m_removePushButton = new QPushButton(i18n("&Remove"), this);
		if (vlayout && m_addPushButton && m_moveUpPushButton &&
				m_moveDownPushButton && m_editPushButton && m_removePushButton) {
			vlayout->addWidget(m_addPushButton);
			vlayout->addWidget(m_moveUpPushButton);
			vlayout->addWidget(m_moveDownPushButton);
			vlayout->addWidget(m_editPushButton);
			vlayout->addWidget(m_removePushButton);
			vlayout->addStretch();

			connect(m_addPushButton, SIGNAL(clicked()), this, SLOT(addItem()));
			connect(m_moveUpPushButton, SIGNAL(clicked()), this, SLOT(moveUpItem()));
			connect(m_moveDownPushButton, SIGNAL(clicked()), this, SLOT(moveDownItem()));
			connect(m_editPushButton, SIGNAL(clicked()), this, SLOT(editItem()));
			connect(m_removePushButton, SIGNAL(clicked()), this, SLOT(removeItem()));
#if QT_VERSION >= 0x040000
			connect(m_stringListBox, SIGNAL(currentChanged(Q3ListBoxItem*)), this, SLOT(setButtonEnableState()));
			connect(m_stringListBox, SIGNAL(doubleClicked(Q3ListBoxItem*)), this, SLOT(editItem()));
			connect(m_stringListBox, SIGNAL(returnPressed(Q3ListBoxItem*)), this, SLOT(editItem()));
#else
			connect(m_stringListBox, SIGNAL(currentChanged(QListBoxItem*)), this, SLOT(setButtonEnableState()));
			connect(m_stringListBox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(editItem()));
			connect(m_stringListBox, SIGNAL(returnPressed(QListBoxItem*)), this, SLOT(editItem()));
#endif

			setButtonEnableState();
		}
	}
}

/**
 * Destructor.
 */
StringListEdit::~StringListEdit()
{
}

/**
 * Set the string list in the list box.
 *
 * @param strList string list
 */
void StringListEdit::setStrings(const QStringList& strList)
{
	m_stringListBox->clear();
	m_stringListBox->insertStringList(strList);
}

/**
 * Store the string list from the list box.
 *
 * @param strList the string list is stored here
 */
void StringListEdit::getStrings(QStringList& strList) const
{
	strList.clear();
	QListBoxItem* item = m_stringListBox->firstItem();
	while (item) {
		strList.append(item->text());
		item = item->next();
	}
}

/**
 * Add a new item.
 */
void StringListEdit::addItem()
{
	bool ok;
	QString txt = QInputDialog::getText(
		i18n("Add Item"), QString::null, QLineEdit::Normal,
		QString::null, &ok, this);
	if (ok && !txt.isEmpty()) {
		m_stringListBox->insertItem(txt);
	}
}

/**
 * Remove the selected item.
 */
void StringListEdit::removeItem()
{
	int idx = m_stringListBox->currentItem();
	if (idx >= 0 && m_stringListBox->isSelected(idx)) {
		m_stringListBox->removeItem(idx);
		if (idx < static_cast<int>(m_stringListBox->count())) {
			m_stringListBox->setSelected(idx, true);
		} else if (idx > 0 && idx - 1 < static_cast<int>(m_stringListBox->count())) {
			m_stringListBox->setSelected(idx - 1, true);
		}
	}
}

/**
 * Edit the selected item.
 */
void StringListEdit::editItem()
{
	int idx = m_stringListBox->currentItem();
	if (idx >= 0 && m_stringListBox->isSelected(idx)) {
		bool ok;
		QString txt = QInputDialog::getText(
			i18n("Edit Item"), QString::null, QLineEdit::Normal,
			m_stringListBox->text(idx), &ok, this);
		if (ok && !txt.isEmpty()) {
			m_stringListBox->changeItem(txt, idx);
		}
	}
}

/**
 * Move the selected item up.
 */
void StringListEdit::moveUpItem()
{
	int idx = m_stringListBox->currentItem();
	if (idx > 0 && m_stringListBox->isSelected(idx)) {
		QString txt = m_stringListBox->text(idx);
		m_stringListBox->removeItem(idx);
		m_stringListBox->insertItem(txt, idx - 1);
		m_stringListBox->setSelected(idx - 1, true);
	}
}

/**
 * Move the selected item down.
 */
void StringListEdit::moveDownItem()
{
	int idx = m_stringListBox->currentItem();
	if (idx >= 0 && idx < static_cast<int>(m_stringListBox->count()) - 1 &&
			m_stringListBox->isSelected(idx)) {
		QString txt = m_stringListBox->text(idx);
		m_stringListBox->removeItem(idx);
		m_stringListBox->insertItem(txt, idx + 1);
		m_stringListBox->setSelected(idx + 1, true);
	}
}

/**
 * Change state of buttons according to the current item and the count.
 */
void StringListEdit::setButtonEnableState()
{
	int idx = m_stringListBox->currentItem();
	if (!m_stringListBox->isSelected(idx)) idx = -1;
	m_moveUpPushButton->setEnabled(idx > 0);
	m_moveDownPushButton->setEnabled(
		idx >= 0 &&
		idx < static_cast<int>(m_stringListBox->count()) - 1);
	m_editPushButton->setEnabled(idx >= 0);
	m_removePushButton->setEnabled(idx >= 0);
}
