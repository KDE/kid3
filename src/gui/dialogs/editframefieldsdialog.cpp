/**
 * \file editframefieldsdialog.cpp
 * Field edit dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#include "editframefieldsdialog.h"
#include <QPushButton>
#include <QImage>
#include <QClipboard>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QBuffer>
#include <QVBoxLayout>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>
#include "kid3application.h"
#include "imageviewer.h"
#include "taggedfile.h"
#include "config.h"
#include "fileconfig.h"
#include "iplatformtools.h"
#include "timeeventmodel.h"
#include "timeeventeditor.h"
#include "chaptereditor.h"
#include "tableofcontentseditor.h"
#include "subframeseditor.h"
#include "pictureframe.h"

/** QTextEdit with label above */
class LabeledTextEdit : public QWidget {
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  LabeledTextEdit(QWidget* parent);

  /**
   * Destructor.
   */
  virtual ~LabeledTextEdit() override;

  /**
   * Get text.
   *
   * @return text.
   */
  QString text() const {
    return m_edit->toPlainText();
  }

  /**
   * Set text.
   *
   * @param txt text
   */
  void setText(const QString& txt) {
    m_edit->setPlainText(txt);
  }

  /**
   * Set focus to text field.
   */
  void setFocus() {
    m_edit->setFocus();
  }

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

private:
  /** Label above edit */
  QLabel* m_label;
  /** Text editor */
  QTextEdit* m_edit;
};


/** LineEdit with label above */
class LabeledLineEdit : public QWidget {
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  LabeledLineEdit(QWidget* parent);

  /**
   * Destructor.
   */
  virtual ~LabeledLineEdit() override;

  /**
   * Get text.
   *
   * @return text.
   */
  QString text() const { return m_edit->text(); }

  /**
   * Set text.
   *
   * @param txt text
   */
  void setText(const QString& txt) { m_edit->setText(txt); }

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

private:
  /** Label above edit */
  QLabel* m_label;
  /** Line editor */
  QLineEdit* m_edit;
};


/** Combo box with label above */
class LabeledComboBox : public QWidget {
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   * @param strlst list with ComboBox items, terminated by NULL
   */
  LabeledComboBox(QWidget* parent, const char* const* strlst);

  /**
   * Destructor.
   */
  virtual ~LabeledComboBox() override;

  /**
   * Get index of selected item.
   *
   * @return index.
   */
  int currentItem() const {
    return m_combo->currentIndex();
  }

  /**
   * Set index of selected item.
   *
   * @param idx index
   */
  void setCurrentItem(int idx) {
    m_combo->setCurrentIndex(idx);
  }

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

private:
  /** Label above combo box */
  QLabel* m_label;
  /** Combo box */
  QComboBox* m_combo;
};


/** QSpinBox with label above */
class LabeledSpinBox : public QWidget {
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  LabeledSpinBox(QWidget* parent);

  /**
   * Destructor.
   */
  virtual ~LabeledSpinBox() override;

  /**
   * Get value.
   *
   * @return text.
   */
  int value() const { return m_spinbox->value(); }

  /**
   * Set value.
   *
   * @param value value
   */
  void setValue(int value) { m_spinbox->setValue(value); }

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

private:
  /** Label above edit */
  QLabel* m_label;
  /** Text editor */
  QSpinBox* m_spinbox;
};


/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledTextEdit::LabeledTextEdit(QWidget* parent) :
  QWidget(parent)
{
  setObjectName(QLatin1String("LabeledTextEdit"));
  auto layout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  m_edit = new QTextEdit(this);
  layout->setContentsMargins(0, 0, 0, 0);
  m_edit->setAcceptRichText(false);
  layout->addWidget(m_label);
  layout->addWidget(m_edit);
}

/**
 * Destructor.
 */
LabeledTextEdit::~LabeledTextEdit()
{
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledLineEdit::LabeledLineEdit(QWidget* parent) :
  QWidget(parent)
{
  setObjectName(QLatin1String("LabeledLineEdit"));
  auto layout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  m_edit = new QLineEdit(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_label);
  layout->addWidget(m_edit);
}

/**
 * Destructor.
 */
LabeledLineEdit::~LabeledLineEdit()
{
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param strlst list with ComboBox items, terminated by NULL
 */
LabeledComboBox::LabeledComboBox(QWidget* parent,
         const char* const* strlst) : QWidget(parent)
{
  setObjectName(QLatin1String("LabeledComboBox"));
  auto layout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  m_combo = new QComboBox(this);
  layout->setContentsMargins(0, 0, 0, 0);
  QStringList strList;
  while (*strlst) {
    strList += QCoreApplication::translate("@default", *strlst++);
  }
  m_combo->addItems(strList);
  layout->addWidget(m_label);
  layout->addWidget(m_combo);
}

/**
 * Destructor.
 */
LabeledComboBox::~LabeledComboBox()
{
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledSpinBox::LabeledSpinBox(QWidget* parent) :
  QWidget(parent)
{
  setObjectName(QLatin1String("LabeledSpinBox"));
  auto layout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  m_spinbox = new QSpinBox(this);
  if (layout && m_label && m_spinbox) {
    m_spinbox->setRange(0, INT_MAX);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    layout->addWidget(m_spinbox);
  }
}

/**
 * Destructor.
 */
LabeledSpinBox::~LabeledSpinBox()
{
}


/**
 * Destructor.
 */
FieldControl::~FieldControl()
{
}


/** Base class for MP3 field controls */
class Mp3FieldControl : public FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  Mp3FieldControl(Frame::Field& field) :
    m_field(field) {}

  /**
   * Destructor.
   */
  virtual ~Mp3FieldControl() override;

protected:
  /** field */
  Frame::Field& m_field;
};

/**
 * Destructor.
 */
Mp3FieldControl::~Mp3FieldControl()
{
}

/** Control to edit standard UTF text fields */
class TextFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  TextFieldControl(Frame::Field& field) :
    Mp3FieldControl(field), m_edit(nullptr) {}

  /**
   * Destructor.
   */
  virtual ~TextFieldControl() override {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

protected:
  /** Text editor widget */
  LabeledTextEdit* m_edit;
};

/** Control to edit single line text fields */
class LineFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  LineFieldControl(Frame::Field& field) :
    Mp3FieldControl(field), m_edit(nullptr) {}

  /**
   * Destructor.
   */
  virtual ~LineFieldControl() override {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

protected:
  /** Line editor widget */
  LabeledLineEdit* m_edit;
};

/** Control to edit integer fields */
class IntFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  IntFieldControl(Frame::Field& field) :
    Mp3FieldControl(field), m_numInp(nullptr) {}

  /**
   * Destructor.
   */
  virtual ~IntFieldControl() override {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

protected:
  /** Spin box widget */
  LabeledSpinBox* m_numInp;
};

/** Control to edit integer fields using a combo box with given values */
class IntComboBoxControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   * @param lst list of strings with possible selections, NULL terminated
   */
  IntComboBoxControl(Frame::Field& field,
                     const char* const* lst) :
    Mp3FieldControl(field), m_ptInp(nullptr), m_strLst(lst) {}

  /**
   * Destructor.
   */
  virtual ~IntComboBoxControl() override {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

protected:
  /** Combo box widget */
  LabeledComboBox* m_ptInp;
  /** List of strings with possible selections */
  const char* const* m_strLst;
};

/** Control to import, export and view data from binary fields */
class BinFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param platformTools platform tools
   * @param app application context
   * @param field      field to edit
   * @param frame      frame with fields to edit
   * @param taggedFile file
   */
  BinFieldControl(IPlatformTools* platformTools, Kid3Application* app,
                  Frame::Field& field,
                  const Frame& frame, const TaggedFile* taggedFile) :
    Mp3FieldControl(field), m_platformTools(platformTools), m_app(app),
    m_bos(nullptr), m_frame(frame), m_taggedFile(taggedFile) {}

  /**
   * Destructor.
   */
  virtual ~BinFieldControl() override {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

protected:
  /** Platform dependent tools */
  IPlatformTools* m_platformTools;
  /** Application context */
  Kid3Application* m_app;
  /** Import, Export, View buttons */
  BinaryOpenSave* m_bos;
  /** frame with fields to edit */
  const Frame& m_frame;
  /** tagged file */
  const TaggedFile* m_taggedFile;
};

/**
 * Control to edit time event fields (synchronized lyrics and event timing
 * codes).
 */
class TimeEventFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param platformTools platform tools
   * @param app application context
   * @param field field to edit
   * @param fields fields of frame to edit
   * @param taggedFile file
   * @param tagNr tag number
   * @param type SynchronizedLyrics or EventTimingCodes
   */
  TimeEventFieldControl(IPlatformTools* platformTools,
                        Kid3Application* app, Frame::Field& field,
                        Frame::FieldList& fields, const TaggedFile* taggedFile,
                        Frame::TagNumber tagNr, TimeEventModel::Type type);

  /**
   * Destructor.
   */
  virtual ~TimeEventFieldControl() override;

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

protected:
  /** Platform dependent tools */
  IPlatformTools* m_platformTools;
  /** application context */
  Kid3Application* m_app;
  /** frame with fields to edit */
  Frame::FieldList& m_fields;
  /** tagged file */
  const TaggedFile* m_taggedFile;
  /** number of edited tag */
  Frame::TagNumber m_tagNr;
  /** item model */
  TimeEventModel* m_model;
  /** editor widget */
  TimeEventEditor* m_editor;
};

/** Control to edit a subframe */
class SubframeFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   */
  SubframeFieldControl(IPlatformTools* platformTools,
                       Kid3Application* app, const TaggedFile* taggedFile,
                       Frame::TagNumber tagNr,
                       Frame::FieldList& fields,
                       Frame::FieldList::iterator begin,
                       Frame::FieldList::iterator end);

  /**
   * Destructor.
   */
  virtual ~SubframeFieldControl() override;

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

private:
  IPlatformTools* m_platformTools;
  Kid3Application* m_app;
  const TaggedFile* m_taggedFile;
  Frame::TagNumber m_tagNr;
  Frame::FieldList& m_fields;
  Frame::FieldList::iterator m_begin;
  Frame::FieldList::iterator m_end;
  SubframesEditor* m_editor;
};

/** Control to edit a chapter */
class ChapterFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  ChapterFieldControl(Frame::Field& field);

  /**
   * Destructor.
   */
  virtual ~ChapterFieldControl() override;

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

private:
  ChapterEditor* m_editor;
};

/** Control to edit table of contents. */
class TableOfContentsFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  TableOfContentsFieldControl(Frame::Field& field);

  /**
   * Destructor.
   */
  virtual ~TableOfContentsFieldControl() override;

  /**
   * Update field from data in field control.
   */
  virtual void updateTag() override;

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent) override;

private:
  TableOfContentsEditor* m_editor;
};


/**
 * Constructor.
 *
 * @param platformTools platform tools
 * @param app application context
 * @param parent parent widget
 * @param field  field containing binary data
 */
BinaryOpenSave::BinaryOpenSave(IPlatformTools* platformTools,
                               Kid3Application* app,
                               QWidget* parent, const Frame::Field& field) :
  QWidget(parent),
  m_platformTools(platformTools), m_app(app),
  m_byteArray(field.m_value.toByteArray()), m_isChanged(false)
{
  setObjectName(QLatin1String("BinaryOpenSave"));
  auto layout = new QHBoxLayout(this);
  m_label = new QLabel(this);
  m_clipButton = new QPushButton(tr("From Clip&board"), this);
  QPushButton* toClipboardButton = new QPushButton(tr("&To Clipboard"), this);
  QPushButton* openButton = new QPushButton(tr("&Import..."), this);
  QPushButton* saveButton = new QPushButton(tr("&Export..."), this);
  QPushButton* viewButton = new QPushButton(tr("&View..."), this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_label);
  layout->addWidget(m_clipButton);
  layout->addWidget(toClipboardButton);
  layout->addWidget(openButton);
  layout->addWidget(saveButton);
  layout->addWidget(viewButton);
  connect(m_clipButton, SIGNAL(clicked()), this, SLOT(clipData()));
  connect(toClipboardButton, SIGNAL(clicked()), this, SLOT(copyData()));
  connect(openButton, SIGNAL(clicked()), this, SLOT(loadData()));
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveData()));
  connect(viewButton, SIGNAL(clicked()), this, SLOT(viewData()));
  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(setClipButtonState()));
  setClipButtonState();
}

/**
 * Enable the "From Clipboard" button if the clipboard contains an image.
 */
void BinaryOpenSave::setClipButtonState()
{
  QClipboard* cb = QApplication::clipboard();
  m_clipButton->setEnabled(
    cb && (cb->mimeData()->hasFormat(QLatin1String("image/jpeg")) ||
           cb->mimeData()->hasImage()));
}

/**
 * Load image from clipboard.
 */
void BinaryOpenSave::clipData()
{
  QClipboard* cb = QApplication::clipboard();
  if (cb) {
    if (cb->mimeData()->hasFormat(QLatin1String("image/jpeg"))) {
      m_byteArray = cb->mimeData()->data(QLatin1String("image/jpeg"));
      m_isChanged = true;
    } else if (cb->mimeData()->hasImage()) {
      QBuffer buffer(&m_byteArray);
      buffer.open(QIODevice::WriteOnly);
      cb->image().save(&buffer, "JPG");
      m_isChanged = true;
    }
  }
}

/**
 * Request name of file to import binary data from.
 * The data is imported later when Ok is pressed in the parent dialog.
 */
void BinaryOpenSave::loadData()
{
  QString loadfilename = m_platformTools->getOpenFileName(this, QString(),
        m_defaultDir.isEmpty() ? m_app->getDirName() : m_defaultDir,
        m_filter, nullptr);
  if (!loadfilename.isEmpty()) {
    QFile file(loadfilename);
    if (file.open(QIODevice::ReadOnly)) {
      int size = file.size();
      auto data = new char[size];
      QDataStream stream(&file);
      stream.readRawData(data, size);
      m_byteArray = QByteArray(data, size);
      m_isChanged = true;
      delete [] data;
      file.close();
    }
  }
}

/**
 * Request name of file and export binary data.
 */
void BinaryOpenSave::saveData()
{
  QString dir = m_defaultDir.isEmpty() ? m_app->getDirName() : m_defaultDir;
  QString fileName(m_defaultFile);
  if (fileName.isEmpty()) {
    fileName = QLatin1String("untitled");
  }
  QChar separator = QDir::separator();
  if (!dir.endsWith(separator)) {
    dir += separator;
  }
  QFileInfo fileInfo(fileName);
  dir += fileInfo.completeBaseName();
  QMimeDatabase mimeDb;
  QString suffix = mimeDb.mimeTypeForData(m_byteArray).preferredSuffix();
  if (suffix == QLatin1String("jpeg")) {
    suffix = QLatin1String("jpg");
  }
  if (!suffix.isEmpty()) {
    dir += QLatin1Char('.');
    dir += suffix;
  }
  QString fn = m_platformTools->getSaveFileName(
        this, QString(), dir, m_filter, nullptr);
  if (!fn.isEmpty()) {
    QFile file(fn);
    if (file.open(QIODevice::WriteOnly)) {
      QDataStream stream(&file);
      stream.writeRawData(m_byteArray.data(), m_byteArray.size());
      file.close();
    }
  }
}

/**
 * Create image from binary data and copy it to clipboard.
 */
void BinaryOpenSave::copyData()
{
  QClipboard* cb = QApplication::clipboard();
  if (cb) {
    QImage image;
    if (image.loadFromData(m_byteArray)) {
      cb->setImage(image, QClipboard::Clipboard);
    } else {
      QMimeDatabase mimeDb;
      QString mimeType = mimeDb.mimeTypeForData(m_byteArray).name();
      if (!mimeType.isEmpty()) {
        auto mimeData = new QMimeData;
        mimeData->setData(mimeType, m_byteArray);
        cb->setMimeData(mimeData);
      }
    }
  }
}

/**
 * Create image from binary data and display it in window.
 */
void BinaryOpenSave::viewData()
{
  QImage image;
  if (image.loadFromData(m_byteArray)) {
    ImageViewer iv(this, image);
    iv.exec();
  }
}

/**
 * Update field with data from dialog.
 */
void TextFieldControl::updateTag()
{
  m_field.m_value = m_edit->text();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TextFieldControl::createWidget(QWidget* parent)
{
  m_edit = new LabeledTextEdit(parent);
  if (m_edit == nullptr)
    return nullptr;

  m_edit->setLabel(Frame::Field::getFieldIdName(
                     static_cast<Frame::FieldId>(m_field.m_id)));
  m_edit->setText(m_field.m_value.toString());
  m_edit->setFocus();
  return m_edit;
}

/**
 * Update field with data from dialog.
 */
void LineFieldControl::updateTag()
{
  m_field.m_value = m_edit->text();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* LineFieldControl::createWidget(QWidget* parent)
{
  m_edit = new LabeledLineEdit(parent);
  m_edit->setLabel(Frame::Field::getFieldIdName(
                     static_cast<Frame::FieldId>(m_field.m_id)));
  m_edit->setText(m_field.m_value.toString());
  return m_edit;
}

/**
 * Update field with data from dialog.
 */
void IntFieldControl::updateTag()
{
  m_field.m_value = m_numInp->value();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* IntFieldControl::createWidget(QWidget* parent)
{
  m_numInp = new LabeledSpinBox(parent);
  m_numInp->setLabel(Frame::Field::getFieldIdName(
                       static_cast<Frame::FieldId>(m_field.m_id)));
  m_numInp->setValue(m_field.m_value.toInt());
  return m_numInp;
}

/**
 * Update field with data from dialog.
 */
void IntComboBoxControl::updateTag()
{
  m_field.m_value = m_ptInp->currentItem();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* IntComboBoxControl::createWidget(QWidget* parent)
{
  m_ptInp = new LabeledComboBox(parent, m_strLst);
  m_ptInp->setLabel(Frame::Field::getFieldIdName(
                      static_cast<Frame::FieldId>(m_field.m_id)));
  m_ptInp->setCurrentItem(m_field.m_value.toInt());
  return m_ptInp;
}

/**
 * Update field with data from dialog.
 */
void BinFieldControl::updateTag()
{
  if (m_bos && m_bos->isChanged()) {
    m_field.m_value = m_bos->getData();
  }
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* BinFieldControl::createWidget(QWidget* parent)
{
  m_bos = new BinaryOpenSave(m_platformTools, m_app, parent, m_field);
  m_bos->setLabel(Frame::Field::getFieldIdName(
                    static_cast<Frame::FieldId>(m_field.m_id)));
  if (m_taggedFile) {
    m_bos->setDefaultDir(m_taggedFile->getDirname());
  }
  if (m_frame.getType() == Frame::FT_Picture) {
    m_bos->setDefaultFile(FileConfig::instance().defaultCoverFileName());
    const char* const imagesStr = QT_TRANSLATE_NOOP("@default", "Images");
    const char* const allFilesStr = QT_TRANSLATE_NOOP("@default", "All Files");
    m_bos->setFilter(m_platformTools->fileDialogNameFilter(
               QList<QPair<QString, QString> >()
               << qMakePair(QCoreApplication::translate("@default", imagesStr),
                            QString(QLatin1String("*.jpg *.jpeg *.png")))
               << qMakePair(QCoreApplication::translate("@default",
                                                        allFilesStr),
                            QString(QLatin1Char('*')))));
  }
  return m_bos;
}

/**
 * Constructor.
 * @param platformTools platform tools
 * @param app application context
 * @param field field to edit
 * @param fields fields of frame to edit
 * @param taggedFile file
 * @param tagNr tag number
 * @param type SynchronizedLyrics or EventTimingCodes
 */
TimeEventFieldControl::TimeEventFieldControl(
    IPlatformTools* platformTools, Kid3Application* app, Frame::Field& field,
    Frame::FieldList& fields, const TaggedFile* taggedFile,
    Frame::TagNumber tagNr, TimeEventModel::Type type) :
  Mp3FieldControl(field), m_platformTools(platformTools), m_app(app),
  m_fields(fields), m_taggedFile(taggedFile), m_tagNr(tagNr),
  m_model(new TimeEventModel(this)), m_editor(nullptr)
{
  m_model->setType(type);
  if (type == TimeEventModel::EventTimingCodes) {
    m_model->fromEtcoFrame(m_fields);
  } else {
    m_model->fromSyltFrame(m_fields);
  }
}

/**
 * Destructor.
 */
TimeEventFieldControl::~TimeEventFieldControl()
{
}

/**
 * Update field with data from dialog.
 */
void TimeEventFieldControl::updateTag()
{
  if (m_model->getType() == TimeEventModel::EventTimingCodes) {
    m_model->toEtcoFrame(m_fields);
  } else {
    m_model->toSyltFrame(m_fields);
  }
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TimeEventFieldControl::createWidget(QWidget* parent)
{
  m_editor = new TimeEventEditor(m_platformTools, m_app, parent,
                                 m_field, m_taggedFile, m_tagNr);
  m_editor->setModel(m_model);
  return m_editor;
}

/**
 * Constructor.
 */
SubframeFieldControl::SubframeFieldControl(
    IPlatformTools* platformTools, Kid3Application* app,
    const TaggedFile* taggedFile, Frame::TagNumber tagNr, Frame::FieldList& fields,
    Frame::FieldList::iterator begin, Frame::FieldList::iterator end) :
  Mp3FieldControl(*begin), m_platformTools(platformTools), m_app(app),
  m_taggedFile(taggedFile), m_tagNr(tagNr), m_fields(fields),
  m_begin(begin), m_end(end), m_editor(nullptr)
{
}

/**
 * Destructor.
 */
SubframeFieldControl::~SubframeFieldControl()
{
}

/**
 * Update field from data in field control.
 */
void SubframeFieldControl::updateTag()
{
  if (m_editor) {
    FrameCollection frames;
    m_editor->getFrames(frames);
    m_fields.erase(m_begin, m_end);
    Frame::Field field;
    field.m_id = Frame::ID_Subframe;
    for (auto it = frames.begin(); it != frames.end(); ++it) {
      field.m_value = it->getExtendedType().getName();
      m_fields.append(field);
      m_fields.append(it->getFieldList());
    }
  }
}

/**
 * Create widget to edit field data.
 *
 * @param parent parent widget
 *
 * @return widget to edit field data.
 */
QWidget* SubframeFieldControl::createWidget(QWidget* parent) {
  m_editor = new SubframesEditor(m_platformTools, m_app, m_taggedFile, m_tagNr,
                                 parent);
  FrameCollection frames = FrameCollection::fromSubframes(
        static_cast<Frame::FieldList::const_iterator>(m_begin),
        static_cast<Frame::FieldList::const_iterator>(m_end));
  m_editor->setFrames(frames);
  return m_editor;
}

/**
 * Constructor.
 * @param field field to edit
 */
ChapterFieldControl::ChapterFieldControl(Frame::Field& field) :
  Mp3FieldControl(field), m_editor(nullptr)
{
}

/**
 * Destructor.
 */
ChapterFieldControl::~ChapterFieldControl()
{
}

/**
 * Update field from data in field control.
 */
void ChapterFieldControl::updateTag()
{
  if (m_editor) {
    quint32 startTimeMs, endTimeMs, startOffset, endOffset;
    m_editor->getValues(startTimeMs, endTimeMs, startOffset, endOffset);
    QVariantList lst;
    lst << startTimeMs << endTimeMs << startOffset << endOffset;
    m_field.m_value = lst;
  }
}

/**
 * Create widget to edit field data.
 *
 * @param parent parent widget
 *
 * @return widget to edit field data.
 */
QWidget* ChapterFieldControl::createWidget(QWidget* parent) {
  m_editor = new ChapterEditor(parent);
  QVariantList lst = m_field.m_value.toList();
  if (lst.size() >= 4) {
    m_editor->setValues(lst.at(0).toUInt(), lst.at(1).toUInt(),
                        lst.at(2).toUInt(), lst.at(3).toUInt());
  }
  return m_editor;
}

/**
 * Constructor.
 * @param field field to edit
 */
TableOfContentsFieldControl::TableOfContentsFieldControl(Frame::Field& field) :
  Mp3FieldControl(field), m_editor(nullptr)
{
}

/**
 * Destructor.
 */
TableOfContentsFieldControl::~TableOfContentsFieldControl()
{
}

/**
 * Update field from data in field control.
 */
void TableOfContentsFieldControl::updateTag()
{
  if (m_editor) {
    bool isTopLevel, isOrdered;
    QStringList elements = m_editor->getValues(isTopLevel, isOrdered);
    QVariantList lst;
    lst << isTopLevel << isOrdered << elements;
    m_field.m_value = lst;
  }
}

/**
 * Create widget to edit field data.
 *
 * @param parent parent widget
 *
 * @return widget to edit field data.
 */
QWidget* TableOfContentsFieldControl::createWidget(QWidget* parent) {
  m_editor = new TableOfContentsEditor(parent);
  QVariantList lst = m_field.m_value.toList();
  if (lst.size() >= 3) {
    m_editor->setValues(lst.at(0).toBool(), lst.at(1).toBool(),
                        lst.at(2).toStringList());
  }
  return m_editor;
}


/**
 * Constructor.
 *
 * @param platformTools platform tools
 * @param app application context
 * @param parent     parent widget
 */
EditFrameFieldsDialog::EditFrameFieldsDialog(IPlatformTools* platformTools,
                                             Kid3Application* app,
                                             QWidget* parent) :
  QDialog(parent), m_platformTools(platformTools), m_app(app)
{
  setObjectName(QLatin1String("EditFrameFieldsDialog"));

#ifdef Q_OS_MAC
  // Make sure that window stays on top, is necessary to keep the dialog
  // visible on Mac OS X while operating with the player for SYLT/ETCO frames.
  setWindowFlags(windowFlags() | Qt::Tool);
#endif

  m_vlayout = new QVBoxLayout(this);

  auto hlayout = new QHBoxLayout;
  QPushButton* okButton = new QPushButton(tr("&OK"));
  QPushButton* cancelButton = new QPushButton(tr("&Cancel"));
  hlayout->addStretch();
  hlayout->addWidget(okButton);
  hlayout->addWidget(cancelButton);
  okButton->setAutoDefault(false);
  cancelButton->setAutoDefault(false);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  m_vlayout->addLayout(hlayout);
  setMinimumWidth(525);
}

/**
 * Destructor.
 */
EditFrameFieldsDialog::~EditFrameFieldsDialog()
{
  qDeleteAll(m_fieldcontrols);
  m_fieldcontrols.clear();
}

/**
 * Set frame to edit.
 *
 * @param frame      frame with fields to edit
 * @param taggedFile file
 * @param tagNr      tag number
 */
void EditFrameFieldsDialog::setFrame(const Frame& frame,
                                     const TaggedFile* taggedFile,
                                     Frame::TagNumber tagNr)
{
  m_fields = frame.getFieldList();

  // Remove all items, keep the last item.
  for (int i = m_vlayout->count() - 2; i >= 0; --i) {
    if (QLayoutItem* item = m_vlayout->takeAt(i)) {
      if (QWidget* widget = item->widget()) {
        delete widget;
      }
      delete item;
    }
  }

  qDeleteAll(m_fieldcontrols);
  m_fieldcontrols.clear();
  bool subframeMissing = false;

  for (auto fldIt = m_fields.begin(); fldIt != m_fields.end(); ++fldIt) {
    Frame::Field& fld = *fldIt;
    if (fld.m_id == Frame::ID_ImageProperties)
      continue;

    if (fld.m_id == Frame::ID_Subframe) {
      SubframeFieldControl* subframeCtl =
          new SubframeFieldControl(m_platformTools, m_app, taggedFile, tagNr,
            m_fields, fldIt, m_fields.end());
      m_fieldcontrols.append(subframeCtl);
      subframeMissing = false;
      break;
    }

    switch (fld.m_value.type()) {
      case QVariant::Int:
      case QVariant::UInt:
        if (fld.m_id == Frame::ID_TextEnc) {
          auto cbox = new IntComboBoxControl(
                fld, Frame::Field::getTextEncodingNames());
          m_fieldcontrols.append(cbox);
        }
        else if (fld.m_id == Frame::ID_PictureType) {
          auto cbox = new IntComboBoxControl(
                fld, PictureFrame::getPictureTypeNames());
          m_fieldcontrols.append(cbox);
        }
        else if (fld.m_id == Frame::ID_TimestampFormat) {
          auto cbox = new IntComboBoxControl(
                fld, Frame::Field::getTimestampFormatNames());
          m_fieldcontrols.append(cbox);
        }
        else if (fld.m_id == Frame::ID_ContentType) {
          auto cbox = new IntComboBoxControl(
                fld, Frame::Field::getContentTypeNames());
          m_fieldcontrols.append(cbox);
        }
        else {
          auto intctl = new IntFieldControl(fld);
          m_fieldcontrols.append(intctl);
        }
        break;

      case QVariant::String:
        if (fld.m_id == Frame::ID_Text) {
          // Large textedit for text fields
          auto textctl = new TextFieldControl(fld);
          m_fieldcontrols.append(textctl);
        }
        else {
          auto textctl = new LineFieldControl(fld);
          m_fieldcontrols.append(textctl);
        }
        break;

      case QVariant::ByteArray:
      {
        auto binctl = new BinFieldControl(
              m_platformTools, m_app, fld, frame, taggedFile);
        m_fieldcontrols.append(binctl);
        break;
      }

      case QVariant::List:
      {
        QString frameName = frame.getName();
        if (frameName.startsWith(QLatin1String("SYLT"))) {
          auto timeEventCtl = new TimeEventFieldControl(
                m_platformTools, m_app, fld, m_fields, taggedFile, tagNr,
                TimeEventModel::SynchronizedLyrics);
          m_fieldcontrols.append(timeEventCtl);
        } else if (frameName.startsWith(QLatin1String("ETCO"))) {
          auto timeEventCtl = new TimeEventFieldControl(
                m_platformTools, m_app, fld, m_fields, taggedFile, tagNr,
                TimeEventModel::EventTimingCodes);
          m_fieldcontrols.append(timeEventCtl);
        } else if (frameName.startsWith(QLatin1String("CHAP"))) {
          auto chapCtl = new ChapterFieldControl(fld);
          m_fieldcontrols.append(chapCtl);
          subframeMissing = true;
        } else if (frameName.startsWith(QLatin1String("CTOC"))) {
          auto tocCtl =
              new TableOfContentsFieldControl(fld);
          m_fieldcontrols.append(tocCtl);
          subframeMissing = true;
        } else {
          qDebug("Unexpected QVariantList in field %d", fld.m_id);
        }
        break;
      }

      default:
        qDebug("Unknown type %d in field %d", fld.m_value.type(), fld.m_id);
    }
  }

  if (subframeMissing) {
    // Add an empty subframe so that subframes can be added
    SubframeFieldControl* subframeCtl =
        new SubframeFieldControl(m_platformTools, m_app, taggedFile, tagNr,
          m_fields, m_fields.end(), m_fields.end());
    m_fieldcontrols.append(subframeCtl);
  }

  // Handle case for frames without fields, just a value.
  m_valueField.m_id = Frame::ID_Text;
  if (m_fields.isEmpty()) {
    m_valueField.m_value = frame.getValue();
    m_fieldcontrols.append(new TextFieldControl(m_valueField));
  } else {
    m_valueField.m_value = QString();
  }

  QListIterator<FieldControl*> it(m_fieldcontrols);
  it.toBack();
  while (it.hasPrevious()) {
    m_vlayout->insertWidget(0, it.previous()->createWidget(this));
  }
}

/**
 * Update fields and get edited fields.
 *
 * @return field list.
 */
const Frame::FieldList& EditFrameFieldsDialog::getUpdatedFieldList()
{
  QListIterator<FieldControl*> it(m_fieldcontrols);
  while (it.hasNext()) {
    it.next()->updateTag();
  }
  return m_fields;
}

/**
 * Get value of frame for frames without a field list.
 * First getUpdatedFieldList() has to be called, if the returned field list
 * is empty, the frame value is available with this method.
 *
 * @return frame value.
 */
QString EditFrameFieldsDialog::getFrameValue() const
{
  return m_valueField.m_value.toString();
}
