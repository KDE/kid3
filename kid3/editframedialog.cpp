/**
 * \file editframedialog.cpp
 * Field edit dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include "editframedialog.h"
#include <QPushButton>
#include <QVBoxLayout>

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption window title
 * @param text    text to edit
 */
EditFrameDialog::EditFrameDialog(QWidget* parent, const QString& caption,
																 const QString& text) :
	QDialog(parent)
{
	setModal(true);
	setWindowTitle(caption);
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (vlayout) {
		vlayout->setSpacing(6);
		vlayout->setMargin(6);
		m_edit = new QTextEdit(this);
		if (m_edit) {
			m_edit->setPlainText(text);
			m_edit->moveCursor(QTextCursor::End);
			vlayout->addWidget(m_edit);
		}
	}
	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
					   QSizePolicy::Minimum);
	m_okButton = new QPushButton(i18n("&OK"), this);
	m_cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && m_okButton && m_cancelButton) {
		hlayout->addItem(hspacer);
		hlayout->addWidget(m_okButton);
		hlayout->addWidget(m_cancelButton);
		m_okButton->setDefault(true);
		connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		vlayout->addLayout(hlayout);
	}
	setMinimumWidth(400);
}

/**
 * Destructor.
 */
EditFrameDialog::~EditFrameDialog() {
}

