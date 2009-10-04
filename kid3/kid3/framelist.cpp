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

#include <qdialog.h>
#include <qinputdialog.h>

#include "taggedfile.h"
#include "framelist.h"
#include "frametable.h"
#include "editframedialog.h"
#include "editframefieldsdialog.h"
#include "kid3.h"
#ifdef CONFIG_USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif


/**
 * Constructor.
 *
 * @param ft frame table
 */
FrameList::FrameList(FrameTable* ft) :
	m_file(0), m_frameTable(ft)
{
}

/**
 * Destructor.
 */
FrameList::~FrameList() {}

/**
 * Clear listbox and file reference.
 */
void FrameList::clear()
{
	m_frameTable->frames().clear();
	m_frameTable->framesToTable();
	m_file = 0;
}

/**
 * Get file containing frames.
 *
 * @return file, NULL if no file selected.
 */
TaggedFile* FrameList::getFile() const
{
	return m_file;
}

/**
 * Reload the frame list, keeping the same row selected.
 */
void FrameList::reloadTags()
{
	m_frameTable->saveCursor();
	setTags(m_file);
	m_frameTable->restoreCursor();
}

/**
 * Get ID of selected frame list item.
 *
 * @return ID of selected item,
 *         -1 if not item is selected.
 */
int FrameList::getSelectedId() const
{
	const Frame* currentFrame = m_frameTable->getCurrentFrame();
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
	const Frame* currentFrame = m_frameTable->getCurrentFrame();
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
	m_frameTable->selectFrameWithIndex(id);
}

/**
 * Get the name of the selected frame.
 *
 * @return name, QString::null if nothing selected.
 */
QString FrameList::getSelectedName() const
{
	const Frame* currentFrame = m_frameTable->getCurrentFrame();
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
	return m_frameTable->selectFrameWithName(name);
}

/**
 * Clear list box.
 */
void FrameList::clearListBox()
{
	if (m_frameTable) {
		m_frameTable->frames().clear();
		m_frameTable->framesToTable();
	}
}

/**
 * Fill listbox with frame descriptions.
 * Before using this method, the listbox and file have to be set.
 * @see setListBox(), setTags()
 */
void FrameList::readTags()
{
	if (m_file) {
		m_file->getAllFramesV2(m_frameTable->frames());
		m_frameTable->framesToTable();
	}
}

/**
 * Set file and fill the list box with its frames.
 * The listbox has to be set with setListBox() before calling this
 * function.
 *
 * @param taggedFile file
 */
void FrameList::setTags(TaggedFile* taggedFile)
{
	m_file = taggedFile;
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
		int nlPos = name.QCM_indexOf("\n");
		if (nlPos > 0) {
			// probably "TXXX - User defined text information\nDescription" or
			// "WXXX - User defined URL link\nDescription"
			name.truncate(nlPos);
		}
#if QT_VERSION >= 0x040000
		name = QCM_translate(name.toLatin1().data());
#else
		name = QCM_translate(name);
#endif
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
			new EditFrameFieldsDialog(0, name, frame, m_file);
		result = dialog && dialog->exec() == QDialog::Accepted;
		if (result) {
			frame.setFieldList(dialog->getUpdatedFieldList());
			frame.setValueFromFieldList();
		}
	}
	if (result && m_file) {
		if (m_file->setFrameV2(frame)) {
			m_file->markTag2Changed(frame.getType());
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
	m_frameTable->saveCursor();
	Frame frame;
	if (getSelectedFrame(frame) && m_file) {
		m_file->deleteFrameV2(frame);
		readTags();
		m_frameTable->restoreCursor();
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
	if (m_file) {
		if (!m_file->addFrameV2(m_frame)) {
			return false;
		}
		if (edit) {
			if (!editFrame(m_frame)) {
				m_file->deleteFrameV2(m_frame);
				m_file->markTag2Unchanged();
				return false;
			}
		}
		int index = m_frame.getIndex();
		readTags(); // refresh listbox
		if (index != -1) {
			setSelectedId(index);
#if QT_VERSION < 0x040000
			m_frameTable->ensureCellVisible(m_frameTable->currentRow(), m_frameTable->currentColumn()); 
#endif
		}
		return true;
	}
	return false;
}

/**
 * Get type of frame from translated name.
 *
 * @param name name, spaces and case are ignored
 *
 * @return type.
 */
static Frame::Type getTypeFromTranslatedName(QString name)
{
	static QMap<QString, int> strNumMap;
	if (strNumMap.empty()) {
		// first time initialization
		for (int i = 0; i <= Frame::FT_LastFrame; ++i) {
			Frame::Type type = static_cast<Frame::Type>(i);
			strNumMap.insert(QCM_translate(Frame::getNameFromType(type)).remove(' ').QCM_toUpper(),
											 type);
		}
	}
	QMap<QString, int>::const_iterator it =
		strNumMap.find(name.remove(' ').QCM_toUpper());
	if (it != strNumMap.end()) {
		return static_cast<Frame::Type>(*it);
	}
	return Frame::FT_Other;
}

/**
 * Display a dialog to select a frame type.
 *
 * @return false if no frame selected.
 */
bool FrameList::selectFrame()
{
	// strange, but necessary to get the strings translated with Qt4 without KDE
	const char* const title = I18N_NOOP("Add Frame");
	const char* const msg = I18N_NOOP("Select the frame ID");
	bool ok = false;
	if (m_file) {
		QString name = QInputDialog::QCM_getItem(
			0, QCM_translate(title),
			QCM_translate(msg), m_file->getFrameIds(), 0, true, &ok);
		if (ok) {
			Frame::Type type = getTypeFromTranslatedName(name);
			m_frame = Frame(type, "", name, -1);
		}
	}
	return ok;
}

/**
 * Paste the selected frame from the copy buffer.
 *
 * @return true if frame pasted.
 */
bool FrameList::pasteFrame() {
	if (m_file && m_frame.getType() != Frame::FT_UnknownFrame) {
		m_file->addFrameV2(m_frame);
		m_file->setFrameV2(m_frame);
		return true;
	}
	return false;
}
