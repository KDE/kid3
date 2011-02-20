/**
 * \file stringlistedit.cpp
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

#include "stringlistedit.h"
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qinputdialog.h>
#include <qlayout.h>
#if QT_VERSION >= 0x040000
#include <QListWidget>
#else
#include <qlistbox.h>
#endif

/**
 * Constructor.
 *
 * @param parent parent widget
 */
StringListEdit::StringListEdit(QWidget* parent) :
	QWidget(parent)
{
	QHBoxLayout* hlayout = new QHBoxLayout(this);
#if QT_VERSION >= 0x040000
	m_stringListBox = new QListWidget(this);
#else
	m_stringListBox = new QListBox(this);
#endif
	if (hlayout && m_stringListBox) {
		hlayout->setSpacing(6);
		hlayout->addWidget(m_stringListBox);
		QVBoxLayout* vlayout = new QVBoxLayout;
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
			connect(m_stringListBox, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(setButtonEnableState()));
			connect(m_stringListBox, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(editItem()));
#else
			connect(m_stringListBox, SIGNAL(currentChanged(QListBoxItem*)), this, SLOT(setButtonEnableState()));
			connect(m_stringListBox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(editItem()));
			connect(m_stringListBox, SIGNAL(returnPressed(QListBoxItem*)), this, SLOT(editItem()));
#endif

			setButtonEnableState();
			hlayout->addLayout(vlayout);
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
	m_stringListBox->QCM_addItems(strList);
}

/**
 * Store the string list from the list box.
 *
 * @param strList the string list is stored here
 */
void StringListEdit::getStrings(QStringList& strList) const
{
	strList.clear();
#if QT_VERSION >= 0x040000
	for (int i = 0; i < m_stringListBox->count(); ++i) {
		strList.append(m_stringListBox->item(i)->text());
	}
#else
	QListBoxItem* item = m_stringListBox->firstItem();
	while (item) {
		strList.append(item->text());
		item = item->next();
	}
#endif
}

/**
 * Add a new item.
 */
void StringListEdit::addItem()
{
	bool ok;
	QString txt = QInputDialog::QCM_getText(
		this, i18n("Add Item"), QString::null, QLineEdit::Normal,
		QString::null, &ok);
	if (ok && !txt.isEmpty()) {
		m_stringListBox->QCM_addItem(txt);
	}
}

/**
 * Remove the selected item.
 */
void StringListEdit::removeItem()
{
#if QT_VERSION >= 0x040000
	int idx = m_stringListBox->currentRow();
	QListWidgetItem* lwi = m_stringListBox->item(idx);
	if (idx >= 0 && lwi) {
		delete m_stringListBox->takeItem(idx);
		if (idx < static_cast<int>(m_stringListBox->count())) {
			m_stringListBox->setCurrentRow(idx);
		} else if (idx > 0 && idx - 1 < static_cast<int>(m_stringListBox->count())) {
			m_stringListBox->setCurrentRow(idx - 1);
		}
		setButtonEnableState();
	}
#else
	int idx = m_stringListBox->currentItem();
	if (idx >= 0 && m_stringListBox->isSelected(idx)) {
		m_stringListBox->removeItem(idx);
		if (idx < static_cast<int>(m_stringListBox->count())) {
			m_stringListBox->setSelected(idx, true);
		} else if (idx > 0 && idx - 1 < static_cast<int>(m_stringListBox->count())) {
			m_stringListBox->setSelected(idx - 1, true);
		}
	}
#endif
}

/**
 * Edit the selected item.
 */
void StringListEdit::editItem()
{
#if QT_VERSION >= 0x040000
	QListWidgetItem* lwi = m_stringListBox->currentItem();
	if (lwi) {
		bool ok;
		QString txt = QInputDialog::getText(
			this, i18n("Edit Item"), QString::null, QLineEdit::Normal,
			lwi->text(), &ok);
		if (ok && !txt.isEmpty()) {
			lwi->setText(txt);
		}
	}
#else
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
#endif
}

/**
 * Move the selected item up.
 */
void StringListEdit::moveUpItem()
{
#if QT_VERSION >= 0x040000
	int idx = m_stringListBox->currentRow();
	QListWidgetItem* lwi = m_stringListBox->item(idx);
	if (idx > 0 && lwi) {
		m_stringListBox->insertItem(idx - 1, m_stringListBox->takeItem(idx));
		m_stringListBox->clearSelection();
		m_stringListBox->setCurrentRow(idx - 1);
	}
#else
	int idx = m_stringListBox->currentItem();
	if (idx > 0 && m_stringListBox->isSelected(idx)) {
		QString txt = m_stringListBox->text(idx);
		m_stringListBox->removeItem(idx);
		m_stringListBox->insertItem(txt, idx - 1);
		m_stringListBox->setSelected(idx - 1, true);
	}
#endif
}

/**
 * Move the selected item down.
 */
void StringListEdit::moveDownItem()
{
#if QT_VERSION >= 0x040000
	int idx = m_stringListBox->currentRow();
	QListWidgetItem* lwi = m_stringListBox->item(idx);
	if (idx >= 0 && idx < static_cast<int>(m_stringListBox->count()) - 1 &&
			lwi) {
		m_stringListBox->insertItem(idx + 1, m_stringListBox->takeItem(idx));
		m_stringListBox->clearSelection();
		m_stringListBox->setCurrentRow(idx + 1);
	}
#else
	int idx = m_stringListBox->currentItem();
	if (idx >= 0 && idx < static_cast<int>(m_stringListBox->count()) - 1 &&
			m_stringListBox->isSelected(idx)) {
		QString txt = m_stringListBox->text(idx);
		m_stringListBox->removeItem(idx);
		m_stringListBox->insertItem(txt, idx + 1);
		m_stringListBox->setSelected(idx + 1, true);
	}
#endif
}

/**
 * Change state of buttons according to the current item and the count.
 */
void StringListEdit::setButtonEnableState()
{
#if QT_VERSION >= 0x040000
	int idx = m_stringListBox->currentRow();
	QListWidgetItem* lwi = m_stringListBox->item(idx);
	if (!lwi) idx = -1;
#else
	int idx = m_stringListBox->currentItem();
	if (!m_stringListBox->isSelected(idx)) idx = -1;
#endif
	m_moveUpPushButton->setEnabled(idx > 0);
	m_moveDownPushButton->setEnabled(
		idx >= 0 &&
		idx < static_cast<int>(m_stringListBox->count()) - 1);
	m_editPushButton->setEnabled(idx >= 0);
	m_removePushButton->setEnabled(idx >= 0);
}
