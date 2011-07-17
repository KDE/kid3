/**
 * \file kid3form.h
 * GUI for kid3.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Apr 2003
 *
 * Copyright (C) 2003-2011  Urs Fleisch
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

#ifndef KID3FORM_H
#define KID3FORM_H

#include <QSplitter>
#include <QLineEdit>
#include <QComboBox>
#include "filelist.h"
#include "dirlist.h"
#include "picturelabel.h"
#include "taggedfile.h"

class QLabel;
class QCheckBox;
class QPushButton;
class QToolButton;
class QSpinBox;
class QGridLayout;
class QGroupBox;
class QPixmap;
class FormatConfig;
class FrameTable;
class FrameTableModel;
class Kid3Application;

/**
 * Main widget.
 */
class Kid3Form : public QSplitter
{
Q_OBJECT

public:
	/** 
	 * Constructs an Id3Form as a child of 'parent', with the 
	 * name 'name' and widget flags set to 'f'.
	 * @param app application
	 * @param parent parent widget
	 */
	explicit Kid3Form(Kid3Application* app, QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~Kid3Form();

	/**
	 * Enable or disable controls requiring ID3v1 tags.
	 *
	 * @param enable true to enable
	 */
	void enableControlsV1(bool enable);

	/**
	 * Display the format of tag 1.
	 *
	 * @param str string describing format, e.g. "ID3v1.1"
	 */
	void setTagFormatV1(const QString& str);

	/**
	 * Display the format of tag 2.
	 *
	 * @param str string describing format, e.g. "ID3v2.4"
	 */
	void setTagFormatV2(const QString& str);

	/**
	 * Adjust the size of the right half box.
	 */
	void adjustRightHalfBoxSize();

	/**
	 * Hide or show file controls.
	 *
	 * @param hide true to hide, false to show
	 */
	void hideFile(bool hide);

	/**
	 * Hide or show tag 1 controls.
	 *
	 * @param hide true to hide, false to show
	 */
	void hideV1(bool hide);

	/**
	 * Hide or show tag 2 controls.
	 *
	 * @param hide true to hide, false to show
	 */
	void hideV2(bool hide);

	/**
	 * Hide or show picture.
	 *
	 * @param hide true to hide, false to show
	 */
	void hidePicture(bool hide);

	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Read the local settings from the configuration.
	 */
	void readConfig();

	/**
	 * Init GUI.
	 */
	void initView();

	/**
	 * Get filename.
	 */
	QString getFilename() const { return m_nameLineEdit->text(); }

	/**
	 * Set filename.
	 * @param fn filename
	 */
	void setFilename(const QString& fn) { m_nameLineEdit->setText(fn); }

	/**
	 * Check if the filename line edit is enabled.
	 * @return true if the filename line edit is enabled.
	 */
	bool isFilenameEditEnabled() const { return m_nameLineEdit->isEnabled(); }

	/**
	 * Enable or disable the filename line edit.
	 * @param en true to enable
	 */
	void setFilenameEditEnabled(bool en) { m_nameLineEdit->setEnabled(en); }

	/**
	 * Mark the filename as changed.
	 * @param en true to mark as changed
	 */
	void markChangedFilename(bool en);

	/**
	 * Set preview picture data.
	 * @param data picture data, 0 if no picture is available
	 */
	void setPictureData(const QByteArray* data) { m_pictureLabel->setData(data); }

	/**
	 * Set details info text.
	 *
	 * @param info detail information
	 */
	void setDetailInfo(const TaggedFile::DetailInfo& info);

	/**
	 * Get directory path.
	 * @return directory path.
	 */
	QString getDirPath() const { return m_fileListBox->getDirPath(); }

	/**
	 * Get file list.
	 * @return file list.
	 */
	FileList* getFileList() { return m_fileListBox; }

	/**
	 * Get tag 1 frame table.
	 * @return frame table.
	 */
	FrameTable* frameTableV1() { return m_framesV1Table; }

	/**
	 * Get tag 2 frame table.
	 * @return frame table.
	 */
	FrameTable* frameTableV2() { return m_framesV2Table; }

public slots:
	/**
	 * Frame list button Edit.
	 */
	void editFrame();

	/**
	 * Frame list button Add.
	 */
	void addFrame();

	/**
	 * Frame list button Delete.
	 */
	void deleteFrame();

	/**
	 * Set filename according to ID3v1 tags.
	 */
	void fnFromID3V1();

	/**
	 * Set filename according to ID3v1 tags.
	 */
	void fnFromID3V2();

	/**
	 * Filename line edit is changed.
	 * @param txt contents of line edit
	 */
	void nameLineEditChanged(const QString& txt);

	/**
	 * Directory list box directory selected.
	 *
	 * @param index selected item
	 */
	void dirSelected(const QModelIndex& index);

	/**
	 * Set focus on filename controls.
	 */
	void setFocusFilename();

	/**
	 * Set focus on tag 1 controls.
	 */
	void setFocusV1();

	/**
	 * Set focus on tag 2 controls.
	 */
	void setFocusV2();

	/**
	 * Select all files.
	 */
	void selectAllFiles();

	/**
	 * Deselect all files.
	 */
	void deselectAllFiles();

	/**
	 * Select first file.
	 *
	 * @return true if a file exists.
	 */
	bool selectFirstFile();

	/**
	 * Select next file.
	 *
	 * @return true if a next file exists.
	 */
	bool selectNextFile();

	/**
	 * Select previous file.
	 *
	 * @return true if a previous file exists.
	 */
	bool selectPreviousFile();

	/**
	 * Set the root index of the directory and file lists.
	 *
	 * @param directoryIndex root index of directory in file system model
	 * @param fileIndex index of file to select
	 */
	void setDirectoryIndex(const QModelIndex& directoryIndex,
												 const QModelIndex& fileIndex);

private:
	/**
	 * Format string within line edit.
	 *
	 * @param le   line edit
	 * @param txt  text in line edit
	 * @param fcfg format configuration
	 */
	void formatLineEdit(QLineEdit* le, const QString& txt,
						const FormatConfig* fcfg);

	FileList* m_fileListBox;
	QComboBox* m_formatComboBox;
	QComboBox* m_formatFromFilenameComboBox;
	QLabel* m_nameLabel;
	QLineEdit* m_nameLineEdit;
	DirList* m_dirListBox;
	FrameTable* m_framesV1Table;
	FrameTable* m_framesV2Table;
	QSplitter* m_vSplitter;
	QWidget* m_fileWidget;
	QWidget* m_tag1Widget;
	QWidget* m_tag2Widget;
	QToolButton* m_fileButton;
	QToolButton* m_tag1Button;
	QToolButton* m_tag2Button;
	QLabel* m_fileLabel;
	QLabel* m_tag1Label;
	QLabel* m_tag2Label;
	QPushButton* m_fnV1Button;
	QPushButton* m_toTagV1Button;
	QPushButton* m_id3V2PushButton;
	QWidget* m_rightHalfVBox;
	PictureLabel* m_pictureLabel;
	Kid3Application* m_app;

	/** Collapse pixmap, will be allocated in constructor */
	static QPixmap* s_collapsePixmap;
	/** Expand pixmap, will be allocated in constructor */
	static QPixmap* s_expandPixmap;

private slots:
	/**
	 * Accept drag.
	 *
	 * @param ev drag event.
	 */
	void dragEnterEvent(QDragEnterEvent* ev);

	/**
	 * Handle drop event.
	 *
	 * @param ev drop event.
	 */
	void dropEvent(QDropEvent* ev);

	/**
	 * Toggle visibility of file controls.
	 */
	void showHideFile();

	/**
	 * Toggle visibility of tag 1 controls.
	 */
	void showHideTag1();

	/**
	 * Toggle visibility of tag 2 controls.
	 */
	void showHideTag2();
};

#endif // KID3FORM_H
