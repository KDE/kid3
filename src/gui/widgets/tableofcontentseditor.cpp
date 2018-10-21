/**
 * \file tableofcontentseditor.cpp
 * Editor for table of contents frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Sep 2015
 *
 * Copyright (C) 2015-2018  Urs Fleisch
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

#include "tableofcontentseditor.h"
#include <QVBoxLayout>
#include <QCheckBox>
#include <QStringListModel>
#include "stringlistedit.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
TableOfContentsEditor::TableOfContentsEditor(QWidget* parent)
  : QWidget(parent)
{
  setObjectName(QLatin1String("TableOfContentsEditor"));
  auto layout = new QVBoxLayout(this);
  m_isTopLevelCheckBox = new QCheckBox(tr("Top level"));
  layout->addWidget(m_isTopLevelCheckBox);
  m_isOrderedCheckBox = new QCheckBox(tr("Ordered"));
  layout->addWidget(m_isOrderedCheckBox);
  m_elementsModel = new QStringListModel(this);
  layout->addWidget(new StringListEdit(m_elementsModel));
}

/**
 * Set chapters in table of contents.
 * @param isTopLevel true if top level
 * @param isOrdered true if contents are ordered
 * @param elements list of child element IDs
 */
void TableOfContentsEditor::setValues(bool isTopLevel, bool isOrdered,
                                      const QStringList& elements)
{
  m_isTopLevelCheckBox->setChecked(isTopLevel);
  m_isOrderedCheckBox->setChecked(isOrdered);
  m_elementsModel->setStringList(elements);
}

/**
 * @brief TableOfContentsEditor::getValues
 * @param isTopLevel true is returned here if top level
 * @param isOrdered true is returned here contents are ordered
 * @return list of child element IDs.
 */
QStringList TableOfContentsEditor::getValues(bool& isTopLevel,
                                             bool& isOrdered) const
{
  isTopLevel = m_isTopLevelCheckBox->isChecked();
  isOrdered = m_isOrderedCheckBox->isChecked();
  return m_elementsModel->stringList();
}
