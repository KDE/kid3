/**
 * \file editframefieldsdialog.h
 * Field edit dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2024  Urs Fleisch
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

#pragma once

#include <QDialog>
#include <QLabel>
#include <QList>
#include "frame.h"

class QVBoxLayout;
class TaggedFile;
class IPlatformTools;
class Kid3Application;
class FieldControl;

/** Row of buttons to load, save and view binary data */
class BinaryOpenSave : public QWidget {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform tools
   * @param app application context
   * @param parent parent widget
   * @param field  field containing binary data
   * @param requiresPicture true if data must be picture
   */
  BinaryOpenSave(IPlatformTools* platformTools, Kid3Application* app,
                 QWidget* parent, const Frame::Field& field,
                 bool requiresPicture);

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

  /**
   * Check if data changed.
   * @return true if data changed.
   */
  bool isChanged() const { return m_isChanged; }

  /**
   * Get binary data.
   * @return byte array.
   */
  const QByteArray& getData() const { return m_byteArray; }

  /**
   * Set default directory name.
   * @param defaultDir default directory name
   */
  void setDefaultDir(const QString& defaultDir) { m_defaultDir = defaultDir; }

  /**
   * Set default file name.
   * @param defaultFile default file name
   */
  void setDefaultFile(const QString& defaultFile) { m_defaultFile = defaultFile; }

  /**
   * Set filter.
   * @param filter filter for file dialog
   */
  void setFilter(const QString& filter) { m_filter = filter; }

public slots:
  /**
   * Enable the "From Clipboard" button if the clipboard contains an image.
   */
  void setClipButtonState();

  /**
   * Load image from clipboard.
   */
  void clipData();

  /**
   * Request name of file to import binary data from.
   * The data is imported later when Ok is pressed in the parent dialog.
   */
  void loadData();

  /**
   * Request name of file and export binary data.
   */
  void saveData();

  /**
   * Create image from binary data and copy it to clipboard.
   */
  void copyData();

  /**
   * Create image from binary data and display it in window.
   */
  void viewData();

private:
  IPlatformTools* m_platformTools;
  Kid3Application* m_app;
  /** Array with binary data */
  QByteArray m_byteArray;
  /** Label left of buttons */
  QLabel* m_label;
  /** From Clipboard button */
  QPushButton* m_clipButton;
  /** Default directory name */
  QString m_defaultDir;
  /** Default file name */
  QString m_defaultFile;
  /** Filter names */
  QString m_filter;
  /** true if m_byteArray changed */
  bool m_isChanged;
  /** true if data must be picture */
  bool m_requiresPicture;
};


/** Field edit dialog */
class EditFrameFieldsDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform tools
   * @param app application context
   * @param parent     parent widget
   */
  EditFrameFieldsDialog(IPlatformTools* platformTools, Kid3Application* app,
                        QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  ~EditFrameFieldsDialog() override;

  /**
   * Set frame to edit.
   *
   * @param frame      frame with fields to edit
   * @param taggedFile file
   * @param tagNr      tag number
   */
  void setFrame(const Frame& frame, const TaggedFile* taggedFile,
                Frame::TagNumber tagNr);

  /**
   * Update fields and get edited fields.
   *
   * @return field list.
   */
  const Frame::FieldList& getUpdatedFieldList();

  /**
   * Get value of frame for frames without a field list.
   * First getUpdatedFieldList() has to be called, if the returned field list
   * is empty, the frame value is available with this method.
   *
   * @return frame value.
   */
  QString getFrameValue() const;

private:
  QVBoxLayout* m_vlayout;
  IPlatformTools* m_platformTools;
  Kid3Application* m_app;
  Frame::FieldList m_fields;
  Frame::Field m_valueField;
  QList<FieldControl*> m_fieldcontrols;
};
