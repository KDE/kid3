/**
 * \file subframeseditor.cpp
 * Editor for subframes contained in a frame.
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

#include "subframeseditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QCoreApplication>
#include "frametablemodel.h"
#include "genremodel.h"
#include "frametable.h"
#include "framelist.h"
#include "taggedfile.h"
#include "editframefieldsdialog.h"
#include "iplatformtools.h"

/**
 * Constructor.
 *
 * @param platformTools platform tools
 * @param app application context
 * @param taggedFile tagged file
 * @param parent parent widget
 */
SubframesEditor::SubframesEditor(IPlatformTools* platformTools,
                                 Kid3Application* app,
                                 const TaggedFile* taggedFile,
                                 Frame::TagNumber tagNr,
                                 QWidget* parent)
  : QWidget(parent), m_platformTools(platformTools), m_app(app),
    m_taggedFile(taggedFile), m_tagNr(tagNr),
    m_editFrameDialog(nullptr), m_editFrameRow(-1)
{
  setObjectName(QLatin1String("SubframesEditor"));
  auto layout = new QHBoxLayout(this);
  m_frameTableModel = new FrameTableModel(
        false, platformTools->iconProvider(), this);
  m_frameTableModel->setHeadersEmpty(true);
  m_frameTable = new FrameTable(m_frameTableModel, new GenreModel(false, this),
                                this);
  layout->addWidget(m_frameTable);
  auto buttonLayout = new QVBoxLayout;
  m_editButton = new QPushButton(tr("Edit..."));
  m_editButton->setDefault(false);
  m_editButton->setAutoDefault(false);
  connect(m_editButton, &QAbstractButton::clicked, this, &SubframesEditor::onEditClicked);
  buttonLayout->addWidget(m_editButton);
  m_addButton = new QPushButton(tr("Add..."));
  m_addButton->setDefault(false);
  m_addButton->setAutoDefault(false);
  connect(m_addButton, &QAbstractButton::clicked, this, &SubframesEditor::onAddClicked);
  buttonLayout->addWidget(m_addButton);
  m_deleteButton = new QPushButton(tr("Delete"));
  m_deleteButton->setDefault(false);
  m_deleteButton->setAutoDefault(false);
  connect(m_deleteButton, &QAbstractButton::clicked, this, &SubframesEditor::onDeleteClicked);
  buttonLayout->addWidget(m_deleteButton);
  buttonLayout->addStretch();
  layout->addLayout(buttonLayout);
}

/**
 * Set subframes.
 * @param frames subframes, will be cleared
 */
void SubframesEditor::setFrames(FrameCollection& frames)
{
  m_frameTableModel->transferFrames(frames);
}

/**
 * Get subframes.
 * @param frames the subframes are returned here
 */
void SubframesEditor::getFrames(FrameCollection& frames) const
{
  frames = m_frameTableModel->frames();
  for (auto it = frames.begin(); it != frames.end(); ++it) {
    auto& frame = const_cast<Frame&>(*it);
    if (frame.isValueChanged()) {
      frame.setFieldListFromValue();
    }
  }
}

/**
 * Called when the Edit button is clicked.
 */
void SubframesEditor::onEditClicked()
{
  QModelIndex index = m_frameTable->currentIndex();
  if (const Frame* selectedFrame = m_frameTableModel->getFrameOfIndex(index)) {
    editFrame(*selectedFrame, index.row());
  }
}

/**
 * Called when the Add button is clicked.
 */
void SubframesEditor::onAddClicked()
{
  bool ok = false;
  QStringList frameIds = m_taggedFile->getFrameIds(m_tagNr);
  QMap<QString, QString> nameMap = Frame::getDisplayNameMap(frameIds);
  QString displayName = QInputDialog::getItem(
    this, tr("Add Frame"),
    tr("Select the frame ID"), nameMap.keys(), 0, true, &ok);
  if (ok) {
    QString name = nameMap.value(displayName, displayName);
    Frame::Type type = Frame::getTypeFromName(name);
    Frame frame(type, QLatin1String(""), name, -1);
    m_taggedFile->addFieldList(m_tagNr, frame);
    editFrame(frame, -1);
  }
}

/**
 * Called when the Delete button is clicked.
 */
void SubframesEditor::onDeleteClicked()
{
  QModelIndex index = m_frameTable->currentIndex();
  if (index.isValid()) {
    m_frameTableModel->removeRow(index.row());
  }
}

/**
 * Let user edit a frame and then update the fields
 * when the edits are accepted.
 *
 * @param frame frame to edit
 * @param row row of edited frame in frame table, -1 if newly added frame
 */
void SubframesEditor::editFrame(const Frame& frame, int row)
{
  m_editFrame = frame;
  if (m_editFrame.isValueChanged()) {
    m_editFrame.setFieldListFromValue();
  }
  m_editFrameRow = row;
  QString name(m_editFrame.getInternalName());
  if (name.isEmpty()) {
    name = m_editFrame.getName();
  }
  if (!name.isEmpty()) {
    int nlPos = name.indexOf(QLatin1Char('\n'));
    if (nlPos > 0) {
      // probably "TXXX - User defined text information\nDescription" or
      // "WXXX - User defined URL link\nDescription"
      name.truncate(nlPos);
    }
    name = QCoreApplication::translate("@default", name.toLatin1().data());
  }
  if (!m_editFrameDialog) {
    m_editFrameDialog = new EditFrameFieldsDialog(m_platformTools, m_app, this);
    connect(m_editFrameDialog, &QDialog::finished,
            this, &SubframesEditor::onEditFrameDialogFinished);
  }
  m_editFrameDialog->setWindowTitle(name);
  m_editFrameDialog->setFrame(m_editFrame, m_taggedFile, m_tagNr);
  m_editFrameDialog->show();
}

/**
 * Called when the edit frame dialog is finished.
 * @param result dialog result
 */
void SubframesEditor::onEditFrameDialogFinished(int result)
{
  if (auto dialog =
      qobject_cast<EditFrameFieldsDialog*>(sender())) {
    if (result == QDialog::Accepted) {
      const Frame::FieldList& fields = dialog->getUpdatedFieldList();
      if (fields.isEmpty()) {
        m_editFrame.setValue(dialog->getFrameValue());
      } else {
        m_editFrame.setFieldList(fields);
        m_editFrame.setValueFromFieldList();
      }
      if (m_editFrameRow != -1) {
        m_frameTableModel->removeRow(m_editFrameRow);
      }
      m_frameTableModel->insertFrame(m_editFrame);
    }
  }
}
