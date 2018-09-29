/**
 * \file kid3form.h
 * GUI for kid3.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Apr 2003
 *
 * Copyright (C) 2003-2017  Urs Fleisch
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
#include "taggedfile.h"
#include "config.h"

class QLabel;
class QCheckBox;
class QPushButton;
class QToolButton;
class QSpinBox;
class QGridLayout;
class QGroupBox;
class QPixmap;
class QComboBox;
class QStackedWidget;
class FormatConfig;
class FrameTable;
class FrameTableModel;
class Kid3Application;
class FileList;
class ConfigurableTreeView;
class PictureLabel;
class BaseMainWindowImpl;
class Kid3FormTagContext;

/**
 * Main widget.
 */
class Kid3Form : public QSplitter {
  Q_OBJECT
public:
  /**
   * Constructs an Id3Form as a child of 'parent', with the
   * name 'name' and widget flags set to 'f'.
   * @param app application
   * @param mainWin main window
   * @param parent parent widget
   */
  Kid3Form(Kid3Application* app, BaseMainWindowImpl* mainWin,
           QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~Kid3Form() override;

  /**
   * Get context for tag.
   * @param tagNr tag number
   * @return tag context.
   */
  Kid3FormTagContext* tag(Frame::TagNumber tagNr) const {
    return m_tagContext[tagNr];
  }

  /**
   * Enable or disable controls requiring tags.
   * @param tagNr tag number
   * @param enable true to enable
   */
  void enableControls(Frame::TagNumber tagNr, bool enable);

  /**
   * Display the tag format.
   * @param tagNr tag number
   * @param str string describing format, e.g. "ID3v1.1"
   */
  void setTagFormat(Frame::TagNumber tagNr, const QString& str);

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
   * Hide or show tag controls.
   * @param tagNr tag number
   * @param hide true to hide, false to show
   */
  void hideTag(Frame::TagNumber tagNr, bool hide);

  /**
   * Toggle visibility of tag controls.
   * @param tagNr tag number
   */
  void showHideTag(Frame::TagNumber tagNr);

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
   * Select in the filename line edit.
   * @param start start position
   * @param length number of characters to select
   */
  void setFilenameSelection(int start, int length) {
    m_nameLineEdit->setSelection(start, length);
    m_nameLineEdit->setFocus();
  }

  /**
   * Mark the filename as changed.
   * @param en true to mark as changed
   */
  void markChangedFilename(bool en);

  /**
   * Set preview picture data.
   * @param data picture data, empty if no picture is available
   */
  void setPictureData(const QByteArray& data);

  /**
   * Set details info text.
   *
   * @param str detail information summary as string
   */
  void setDetailInfo(const QString& str);

  /**
   * Get file list.
   * @return file list.
   */
  FileList* getFileList() { return m_fileListBox; }

  /**
   * Get directory list.
   * @return directory list.
   */
  ConfigurableTreeView* getDirList() { return m_dirListBox; }

  /**
   * Get frame table.
   * @param tagNr tag number
   * @return frame table.
   */
  FrameTable* frameTable(Frame::TagNumber tagNr) { return m_frameTable[tagNr]; }

  /**
   * Set a widget to be displayed at the left side instead of the file lists.
   * @param widget widget to be shown at the left side
   */
  void setLeftSideWidget(QWidget* widget);

  /**
   * Remove widget set with setLeftSideWidget().
   *
   * The widget will not be deleted.
   *
   * @param widget widget to be removed
   */
  void removeLeftSideWidget(QWidget* widget);

public slots:
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
   * Set focus on tag controls.
   * @param tagNr tag number
   */
  void setFocusTag(Frame::TagNumber tagNr);

  /**
   * Set focus on file list.
   */
  void setFocusFileList();

  /**
   * Set focus on directory list.
   */
  void setFocusDirList();

  /**
   * Select all files.
   */
  void selectAllFiles();

  /**
   * Deselect all files.
   */
  void deselectAllFiles();

  /**
   * Set the next file as the current file.
   *
   * @param select true to select the file
   * @param onlyTaggedFiles only consider tagged files
   *
   * @return true if a next file exists.
   */
  bool nextFile(bool select = true, bool onlyTaggedFiles = true);

  /**
   * Set the previous file as the current file.
   *
   * @param select true to select the file
   * @param onlyTaggedFiles only consider tagged files
   *
   * @return true if a previous file exists.
   */
  bool previousFile(bool select = true, bool onlyTaggedFiles = true);

  /**
   * Set the root index of the file list.
   *
   * @param index root index of directory in file system model
   */
  void setFileRootIndex(const QModelIndex& index);

  /**
   * Set the root index of the directory list.
   *
   * @param index root index of directory in directory model
   */
  void setDirRootIndex(const QModelIndex& index);

private slots:
  /**
   * Toggle visibility of file controls.
   */
  void showHideFile();

  /**
   * Set format text configuration when format edit text is changed.
   * @param text format text
   */
  void onFormatEditTextChanged(const QString& text);

  /**
   * Set format from filename text configuration when edit text is changed.
   * @param text format text
   */
  void onFormatFromFilenameEditTextChanged(const QString& text);

  /**
   * Update sorting after directory is opened for the first time.
   * The sort order of the file list is not correct if it is not explicitly
   * sorted the first time.
   */
  void onFirstDirectoryOpened();

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

  /**
   * Get frame table which is currently in editing state.
   * The returned frame table can be used to restore the editing state after
   * changing the current file.
   * @return frame table which is in editing state, 0 if none.
   */
  FrameTable* getEditingFrameTable() const;

  FileList* m_fileListBox;
  QComboBox* m_formatComboBox;
  QComboBox* m_formatFromFilenameComboBox;
  QLabel* m_nameLabel;
  QLineEdit* m_nameLineEdit;
  ConfigurableTreeView* m_dirListBox;
  Kid3FormTagContext* m_tagContext[Frame::Tag_NumValues];
  FrameTable* m_frameTable[Frame::Tag_NumValues];
  QStackedWidget* m_leftSideWidget;
  QSplitter* m_vSplitter;
  QWidget* m_fileWidget;
  QWidget* m_tagWidget[Frame::Tag_NumValues];
  QToolButton* m_fileButton;
  QToolButton* m_tagButton[Frame::Tag_NumValues];
  QLabel* m_fileLabel;
  QLabel* m_tagLabel[Frame::Tag_NumValues];
  QPushButton* m_fnButton[Frame::Tag_NumValues];
  QPushButton* m_toTagButton[Frame::Tag_NumValues];
  QPushButton* m_id3PushButton[Frame::Tag_NumValues];
  QWidget* m_rightHalfVBox;
  PictureLabel* m_pictureLabel;
  Kid3Application* m_app;
  BaseMainWindowImpl* m_mainWin;

  /** Collapse pixmap, will be allocated in constructor */
  static QPixmap* s_collapsePixmap;
  /** Expand pixmap, will be allocated in constructor */
  static QPixmap* s_expandPixmap;
};

/**
 * Facade to have a uniform interface for different tags.
 */
class Kid3FormTagContext : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param form GUI form
   * @param tagNr tag number
   */
  Kid3FormTagContext(Kid3Form* form, Frame::TagNumber tagNr) : QObject(form),
    m_form(form), m_tagNr(tagNr) {
  }

public slots:
  /**
   * Set focus on tag controls.
   */
  void setFocusTag() { m_form->setFocusTag(m_tagNr); }

  /**
   * Toggle visibility of tag controls.
   */
  void showHideTag() { m_form->showHideTag(m_tagNr); }

private:
  Kid3Form* const m_form;
  const Frame::TagNumber m_tagNr;
};

#endif // KID3FORM_H
