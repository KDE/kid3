/**
 * \file framelist.cpp
 * List of ID3v2.3 frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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

#include "framelist.h"
#include <QDialog>
#include <QItemSelectionModel>

#include "taggedfile.h"
#include "frametable.h"
#include "frametablemodel.h"
#include "editframedialog.h"
#include "editframefieldsdialog.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param ftm frame table model
 * @param selModel item selection model
 */
FrameList::FrameList(FrameTableModel* ftm, QItemSelectionModel* selModel) :
	m_taggedFile(0), m_frameTableModel(ftm), m_selectionModel(selModel),
	m_cursorRow(-1), m_cursorColumn(-1)
{
}

/**
 * Destructor.
 */
FrameList::~FrameList() {}

/**
 * Get ID of selected frame list item.
 *
 * @return ID of selected item,
 *         -1 if not item is selected.
 */
int FrameList::getSelectedId() const
{
	const Frame* currentFrame =
		m_frameTableModel->getFrameOfIndex(m_selectionModel->currentIndex());
	return currentFrame ? currentFrame->getIndex() : -1;
}

/**
 * Get frame of selected frame list item.
 *
 * @param frame the selected frame is returned here
 *
 * @return false if not item is selected.
 */
bool FrameList::getSelectedFrame(Frame& frame) const
{
	const Frame* currentFrame =
		m_frameTableModel->getFrameOfIndex(m_selectionModel->currentIndex());
	if (currentFrame) {
		frame = *currentFrame;
		return true;
	}
	return false;
}

/**
 * Select the frame by ID.
 *
 * @param id ID of frame to select
 */
void FrameList::setSelectedId(int id)
{
	m_selectionModel->setCurrentIndex(
		m_frameTableModel->index(
			m_frameTableModel->getRowWithFrameIndex(id), 0),
				QItemSelectionModel::SelectCurrent);
}

/**
 * Get the name of the selected frame.
 *
 * @return name, QString::null if nothing selected.
 */
QString FrameList::getSelectedName() const
{
	const Frame* currentFrame =
		m_frameTableModel->getFrameOfIndex(m_selectionModel->currentIndex());
	return currentFrame ? currentFrame->getName() : QString::null;
}

/**
 * Select a frame with a given name.
 *
 * @param name name of frame
 *
 * @return true if a frame with that name could be selected.
 */
bool FrameList::selectByName(const QString& name)
{
	int row = m_frameTableModel->getRowWithFrameName(name);
	if (row < 0)
		return false;

	m_selectionModel->setCurrentIndex(m_frameTableModel->index(row, 0),
																		QItemSelectionModel::SelectCurrent);
	return true;
}

/**
 * Set the frame table model from the tagged file.
 */
void FrameList::setModelFromTaggedFile()
{
	if (m_taggedFile) {
		FrameCollection frames;
		m_taggedFile->getAllFramesV2(frames);
		m_frameTableModel->transferFrames(frames);
	}
}

/**
 * Create dialog to edit a frame and update the fields
 * if Ok is returned.
 *
 * @param frame frame to edit
 *
 * @return true if Ok selected in dialog.
 */
bool FrameList::editFrame(Frame& frame)
{
	bool result = true;
	QString name(frame.getName(true));
	if (!name.isEmpty()) {
		int nlPos = name.indexOf("\n");
		if (nlPos > 0) {
			// probably "TXXX - User defined text information\nDescription" or
			// "WXXX - User defined URL link\nDescription"
			name.truncate(nlPos);
		}
		name = QCM_translate(name.toLatin1().data());
	}
	if (frame.getFieldList().empty()) {
		EditFrameDialog* dialog =
			new EditFrameDialog(0, name, frame.getValue());
		result = dialog && dialog->exec() == QDialog::Accepted;
		if (result) {
			frame.setValue(dialog->getText());
		}
	} else {
		EditFrameFieldsDialog* dialog =
			new EditFrameFieldsDialog(0, name, frame, m_taggedFile);
		result = dialog && dialog->exec() == QDialog::Accepted;
		if (result) {
			frame.setFieldList(dialog->getUpdatedFieldList());
			frame.setValueFromFieldList();
		}
	}
	if (result && m_taggedFile) {
		if (m_taggedFile->setFrameV2(frame)) {
			m_taggedFile->markTag2Changed(frame.getType());
		}
	}
	return result;
}

/**
 * Create dialog to edit the selected frame and update the fields
 * if Ok is returned.
 *
 * @return true if Ok selected in dialog.
 */
bool FrameList::editFrame()
{
	if (getSelectedFrame(m_frame)) {
		return editFrame(m_frame);
	}
	return false;
}

/**
 * Delete selected frame.
 *
 * @return false if frame not found.
 */
bool FrameList::deleteFrame()
{
	saveCursor();
	Frame frame;
	if (getSelectedFrame(frame) && m_taggedFile) {
		m_taggedFile->deleteFrameV2(frame);
		setModelFromTaggedFile();
		restoreCursor();
		return true;
	}
	return false;
}

/**
 * Add a new frame.
 *
 * @param edit    true to edit frame after adding it
 *
 * @return true if frame added.
 */
bool FrameList::addFrame(bool edit)
{
	if (m_taggedFile) {
		if (!m_taggedFile->addFrameV2(m_frame)) {
			return false;
		}
		if (edit) {
			if (!editFrame(m_frame)) {
				m_taggedFile->deleteFrameV2(m_frame);
				m_taggedFile->markTag2Unchanged();
				return false;
			}
		}
		int index = m_frame.getIndex();
		setModelFromTaggedFile();
		if (index != -1) {
			setSelectedId(index);
		}
		return true;
	}
	return false;
}

/**
 * Paste the selected frame from the copy buffer.
 *
 * @return true if frame pasted.
 */
bool FrameList::pasteFrame() {
	if (m_taggedFile && m_frame.getType() != Frame::FT_UnknownFrame) {
		m_taggedFile->addFrameV2(m_frame);
		m_taggedFile->setFrameV2(m_frame);
		return true;
	}
	return false;
}

/**
 * Save the current cursor position.
 */
void FrameList::saveCursor()
{
	m_cursorRow = m_selectionModel->currentIndex().row();
	m_cursorColumn = m_selectionModel->currentIndex().column();
}

/**
 * Restore the cursor position saved with saveCursor().
 */
void FrameList::restoreCursor()
{
	int lastRow = m_frameTableModel->rowCount() - 1;
	if (m_cursorRow >= 0 && m_cursorColumn >= 0 && lastRow >= 0) {
		if (m_cursorRow > lastRow) {
			m_cursorRow = lastRow;
		}
		m_selectionModel->setCurrentIndex(
			m_frameTableModel->index(m_cursorRow, m_cursorColumn),
			QItemSelectionModel::SelectCurrent);
	}
}
