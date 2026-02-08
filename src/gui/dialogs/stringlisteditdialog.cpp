/**
* \file stringlisteditdialog.cpp
 * Editor to edit a list of strings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Nov 2018
 *
 * Copyright (C) 2018-2024  Urs Fleisch
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

#include "stringlisteditdialog.h"
#include <QStringListModel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include "stringlistedit.h"

StringListEditDialog::StringListEditDialog(
    const QStringList& strings, const QString& title, QWidget* parent)
  : QDialog(parent),
    m_model(new QStringListModel(strings, this)),
    m_formatEdit(new StringListEdit(m_model, this))
{
  setWindowTitle(title);
  auto vlayout = new QVBoxLayout(this);
  vlayout->addWidget(m_formatEdit);
  auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                        QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  vlayout->addWidget(buttonBox);
}

StringListEditDialog::~StringListEditDialog() = default;

QStringList StringListEditDialog::stringList() const {
  return m_model->stringList();
}
