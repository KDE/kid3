/**
 * \file formatlistedit.cpp
 * Widget to edit a format list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Aug 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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

#include "formatlistedit.h"
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>
#include <QFormLayout>
#include <QSizePolicy>
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param labels list of label texts for fields in a single format
 * @param tooltips list of tooltips, one string per label, empty if not used
 * @param parent parent widget
 */
FormatListEdit::FormatListEdit(const QStringList& labels,
                               const QStringList& tooltips,
                               QWidget* parent) :
  QWidget(parent)
{
  setObjectName(QLatin1String("FormatListEdit"));
  QHBoxLayout* hlayout = new QHBoxLayout(this);
  hlayout->setMargin(0);
  QFormLayout* formatLayout = new QFormLayout;
  formatLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  bool comboBoxCreated = false;
  for (int i = 0; i < labels.size(); ++i) {
    const QString& label = labels.at(i);
    const QString& toolTip = tooltips.at(i);
    if (!comboBoxCreated) {
      m_formatComboBox = new QComboBox;
      m_formatComboBox->setEditable(true);
      m_formatComboBox->setInsertPolicy(QComboBox::NoInsert);
      connect(m_formatComboBox, SIGNAL(activated(int)),
              this, SLOT(updateLineEdits(int)));
      connect(m_formatComboBox->lineEdit(), SIGNAL(editingFinished()),
              this, SLOT(commitCurrentEdits()));
      if (!toolTip.isEmpty())
        m_formatComboBox->setToolTip(toolTip);
      formatLayout->addRow(label, m_formatComboBox);
      comboBoxCreated = true;
    } else {
      QLineEdit* ed = new QLineEdit;
      connect(ed, SIGNAL(returnPressed()), this, SIGNAL(formatChanged()));
      if (!toolTip.isEmpty())
        ed->setToolTip(toolTip);
      formatLayout->addRow(label, ed);
      m_lineEdits.append(ed);
    }
  }
  hlayout->addLayout(formatLayout);
  QVBoxLayout* vlayout = new QVBoxLayout;
  vlayout->setSpacing(2);
  m_addPushButton = new QPushButton(i18n("&Add"));
  m_addPushButton->setAutoDefault(false);
  m_removePushButton = new QPushButton(i18n("&Remove"));
  m_removePushButton->setAutoDefault(false);
  vlayout->addWidget(m_addPushButton);
  vlayout->addWidget(m_removePushButton);
  vlayout->addStretch();
  hlayout->addLayout(vlayout);
  connect(m_addPushButton, SIGNAL(clicked()), this, SLOT(addItem()));
  connect(m_removePushButton, SIGNAL(clicked()), this, SLOT(removeItem()));
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

/**
 * Destructor.
 */
FormatListEdit::~FormatListEdit()
{
}

/**
 * Set format strings.
 *
 * @param formats list of format stringlists, the first stringlist contains
 *   the names, the second the corresponding string for the first line edit,
 *   etc.
 * @param index index to select, -1 to keep current index
 */
void FormatListEdit::setFormats(const QList<QStringList>& formats, int index)
{
  m_formats = formats;
  if (index >= 0) {
    m_formatComboBox->setCurrentIndex(index);
    updateComboBoxAndLineEdits(index);
  }
}

/**
 * Get format strings.
 *
 * @param index  if not null, the current index is returned here
 *
 * @return list of format stringlists, the first stringlist contains
 *   the names, the second the corresponding string for the first line edit,
 *   etc.
 */
QList<QStringList> FormatListEdit::getFormats(int* index)
{
  commitCurrentEdits();
  if (index) {
    *index = m_formatComboBox->currentIndex();
  }
  return m_formats;
}

/**
 * Get a format string from the format currently displayed in the GUI.
 *
 * @param formatNr index of the format stringlist, 0 is the format name
 *   1 the first line edit, etc.
 *
 * @return format string.
 */
QString FormatListEdit::getCurrentFormat(int formatNr) const
{
  if (formatNr == 0) {
    return m_formatComboBox->currentText();
  } else if (formatNr > 0 && formatNr - 1 < m_lineEdits.size()) {
    return m_lineEdits.at(formatNr - 1)->text();
  } else {
    return QString();
  }
}

/**
 * Update GUI controls from formats.
 *
 * @param index combo box index to set
 */
void FormatListEdit::updateComboBoxAndLineEdits(int index)
{
  m_formatComboBox->clear();
  if (!m_formats.isEmpty()) {
    m_formatComboBox->addItems(m_formats.first());
    if (index >= 0 && index < m_formats.first().size()) {
      m_formatComboBox->setCurrentIndex(index);
      updateLineEdits(index);
    }
  }
}

/**
 * Set the currently selected format from the contents of the controls.
 */
void FormatListEdit::commitCurrentEdits()
{
  int index = m_formatComboBox->currentIndex();
  if (index < 0)
    return;

  if (m_formatComboBox->itemText(index) != m_formatComboBox->currentText()) {
    m_formatComboBox->setItemText(index,  m_formatComboBox->currentText());
  }

  for (int i = 0; i < m_formats.size() && i - 1 < m_lineEdits.size(); ++i) {
    QString text(i == 0
                 ? m_formatComboBox->currentText()
                 : m_lineEdits.at(i - 1)->text());
    QStringList& fmts = m_formats[i];
    if (index < fmts.size()) {
      fmts[index] = text;
    }
  }
}

/**
 * Set the format lineedits to the format of the index.
 *
 * @param index selected item in combo box
 */
void FormatListEdit::updateLineEdits(int index)
{
  for (int i = 0; i < m_lineEdits.size() && i + 1 < m_formats.size(); ++i) {
    QLineEdit* le = m_lineEdits.at(i);
    const QStringList& fmts = m_formats.at(i + 1);
    if (index < fmts.size()) {
      le->setText(fmts.at(index));
    } else {
      le->clear();
    }
  }
  emit formatChanged();
}

/**
 * Add a new item.
 */
void FormatListEdit::addItem()
{
  commitCurrentEdits();
  if (!m_formats.isEmpty()) {
    // first search for an existing empty format
    int index = -1;
    for (int fmtIdx = m_formats.first().size() - 1; fmtIdx > 0; --fmtIdx) {
      bool allEmpty = true;
      for (int leIdx = 1; leIdx < m_formats.size(); ++leIdx) {
        const QStringList& fmts = m_formats.at(leIdx);
        if (fmtIdx < fmts.size() && !fmts.at(fmtIdx).isEmpty()) {
          allEmpty = false;
          break;
        }
      }
      if (allEmpty) {
        index = fmtIdx;
        break;
      }
    }

    if (index == -1) {
      // no empty format found, add a new one
      for (int i = 0; i < m_formats.size(); ++i) {
        m_formats[i].append(i == 0 ? i18n("New") : QLatin1String(""));
      }
      index = m_formats.first().size() - 1;
    }
    updateComboBoxAndLineEdits(index);
    m_formatComboBox->lineEdit()->setFocus();
    m_formatComboBox->lineEdit()->selectAll();
  }
}

/**
 * Remove the selected item.
 */
void FormatListEdit::removeItem()
{
  int index = m_formatComboBox->currentIndex();
  if (index < 0)
    return;

  for (int i = 0; i < m_formats.size(); ++i) {
    if (index < m_formats[i].size()) {
      m_formats[i].removeAt(index);
    }
  }
  if (!m_formats.isEmpty()) {
    const QStringList& fmts = m_formats.first();
    if (index >= fmts.size()) {
      index = fmts.size() - 1;
    }
    if (index < 0) {
      addItem();
    } else {
      updateComboBoxAndLineEdits(index);
    }
  }
}
