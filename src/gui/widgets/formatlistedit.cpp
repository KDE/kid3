/**
 * \file formatlistedit.cpp
 * Widget to edit a format list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Aug 2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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

/**
 * Constructor.
 *
 * @param labels list of label texts for fields in a single format
 * @param tooltips list of tooltips, one string per label, empty if not used
 * @param parent parent widget
 */
FormatListEdit::FormatListEdit(const QStringList& labels,
                               const QStringList& tooltips,
                               QWidget* parent)
  : QWidget(parent)
{
  setObjectName(QLatin1String("FormatListEdit"));
  auto hlayout = new QHBoxLayout(this);
  hlayout->setContentsMargins(0, 0, 0, 0);
  auto formatLayout = new QFormLayout;
  formatLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  bool comboBoxCreated = false;
  for (int i = 0; i < labels.size(); ++i) {
    const QString& label = labels.at(i);
    const QString& toolTip = tooltips.at(i);
    if (!comboBoxCreated) {
      m_formatComboBox = new QComboBox;
      m_formatComboBox->setEditable(true);
      m_formatComboBox->setInsertPolicy(QComboBox::NoInsert);
      connect(m_formatComboBox, static_cast<void (QComboBox::*)(int)>(
                &QComboBox::activated),
              this, &FormatListEdit::updateLineEdits);
      connect(m_formatComboBox->lineEdit(), &QLineEdit::editingFinished,
              this, &FormatListEdit::commitCurrentEdits);
      if (!toolTip.isEmpty())
        m_formatComboBox->setToolTip(toolTip);
      formatLayout->addRow(label, m_formatComboBox);
      comboBoxCreated = true;
    } else {
      auto ed = new QLineEdit;
      connect(ed, &QLineEdit::returnPressed, this, &FormatListEdit::formatChanged);
      if (!toolTip.isEmpty())
        ed->setToolTip(toolTip);
      formatLayout->addRow(label, ed);
      m_lineEdits.append(ed); // clazy:exclude=reserve-candidates
    }
  }
  hlayout->addLayout(formatLayout);
  auto vlayout = new QVBoxLayout;
#ifdef Q_OS_MAC
  vlayout->setSpacing(6);
#endif
  m_addPushButton = new QPushButton(tr("&Add"));
  m_addPushButton->setAutoDefault(false);
  m_removePushButton = new QPushButton(tr("&Remove"));
  m_removePushButton->setAutoDefault(false);
  vlayout->addWidget(m_addPushButton);
  vlayout->addWidget(m_removePushButton);
  vlayout->addStretch();
  hlayout->addLayout(vlayout);
  connect(m_addPushButton, &QAbstractButton::clicked, this, &FormatListEdit::addItem);
  connect(m_removePushButton, &QAbstractButton::clicked, this, &FormatListEdit::removeItem);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
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
  }
  if (formatNr > 0 && formatNr - 1 < m_lineEdits.size()) {
    return m_lineEdits.at(formatNr - 1)->text();
  }
  return QString();
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
#if QT_VERSION >= 0x050600
    const QStringList& firstFormat = m_formats.constFirst();
#else
    const QStringList& firstFormat = m_formats.first();
#endif
    m_formatComboBox->addItems(firstFormat);
    if (index >= 0 && index < firstFormat.size()) {
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
    if (QStringList& fmts = m_formats[i]; index < fmts.size()) { // clazy:exclude=detaching-member
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
    if (const QStringList& fmts = m_formats.at(i + 1); index < fmts.size()) {
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
#if QT_VERSION >= 0x050600
    for (int fmtIdx = m_formats.constFirst().size() - 1; fmtIdx > 0; --fmtIdx)
#else
    for (int fmtIdx = m_formats.first().size() - 1; fmtIdx > 0; --fmtIdx)
#endif
    {
      bool allEmpty = true;
      for (int leIdx = 1; leIdx < m_formats.size(); ++leIdx) {
        if (const QStringList& fmts = m_formats.at(leIdx);
            fmtIdx < fmts.size() && !fmts.at(fmtIdx).isEmpty()) {
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
        m_formats[i].append(i == 0 ? tr("New") : QLatin1String(""));
      }
#if QT_VERSION >= 0x050600
      index = m_formats.constFirst().size() - 1;
#else
      index = m_formats.first().size() - 1;
#endif
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
    if (index < m_formats.at(i).size()) {
      m_formats[i].removeAt(index);
    }
  }
  if (!m_formats.isEmpty()) {
#if QT_VERSION >= 0x050600
    const QStringList& fmts = m_formats.constFirst();
#else
    const QStringList& fmts = m_formats.first();
#endif
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
