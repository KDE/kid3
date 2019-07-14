/**
 * \file kid3form.cpp
 * GUI for kid3.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Apr 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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

#include "kid3form.h"
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QSpinBox>
#include <QLayout>
#include <QToolTip>
#include <QStackedWidget>
#include <QSplitter>
#include <QDir>
#include <QFrame>
#include <QPixmap>
#include <QComboBox>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QLabel>
#include <QGridLayout>
#include <QScrollArea>
#include <QUrl>
#include <QApplication>
#include <QMimeData>
#include <QMenu>
#include <QFileIconProvider>
#include <QStyle>
#include <QHeaderView>
#include "filesystemmodel.h"
#include "frametable.h"
#include "frametablemodel.h"
#include "trackdata.h"
#include "genres.h"
#include "basemainwindow.h"
#include "filelist.h"
#include "framelist.h"
#include "configurabletreeview.h"
#include "picturelabel.h"
#include "fileconfig.h"
#include "guiconfig.h"
#include "formatconfig.h"
#include "dirproxymodel.h"
#include "fileproxymodel.h"
#include "filesystemmodel.h"
#include "coretaggedfileiconprovider.h"
#include "kid3application.h"
#ifdef Q_OS_MAC
#include <CoreFoundation/CFURL.h>
#endif

/** Collapse pixmap, will be allocated in constructor */
QPixmap* Kid3Form::s_collapsePixmap = nullptr;
/** Expand pixmap, will be allocated in constructor */
QPixmap* Kid3Form::s_expandPixmap = nullptr;

namespace {

/** picture for collapse pixmap */
const char* const collapse_xpm[] = {
  "7 7 3 1",
  " \tc None",
  ".\tc #FFFFFF",
  "+\tc #000000",
  ".......",
  ".......",
  ".......",
  ".+++++.",
  ".......",
  ".......",
  "......."
};

/** picture for expand pixmap */
const char* const expand_xpm[] = {
  "7 7 3 1",
  " \tc None",
  ".\tc #FFFFFF",
  "+\tc #000000",
  ".......",
  "...+...",
  "...+...",
  ".+++++.",
  "...+...",
  "...+...",
  "......."
};


/**
 * Event filter for double click on picture label.
 */
class PictureDblClickHandler : public QObject {
public:
  /**
   * Constructor.
   * @param app application
   */
  explicit PictureDblClickHandler(Kid3Application* app)
    : QObject(app), m_app(app) {}
  virtual ~PictureDblClickHandler() override = default;

protected:
  /**
   * Event filter function, calls Kid3Application::editOrAddPicture().
   *
   * @param obj watched object
   * @param event event for object
   *
   * @return true if event is filtered.
   */
  virtual bool eventFilter(QObject* obj, QEvent* event) override;

private:
  Q_DISABLE_COPY(PictureDblClickHandler)

  Kid3Application* m_app;
};

/**
 * Event filter function, calls Kid3Application::editOrAddPicture() on double click.
 *
 * @param obj watched object
 * @param event event for object
 *
 * @return true if event is filtered.
 */
bool PictureDblClickHandler::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonDblClick) {
    m_app->editOrAddPicture();
    return true;
  } else {
    // standard event processing
    return QObject::eventFilter(obj, event);
  }
}


/**
 * File decoration provider for FileSystemModel delegating to QFileIconProvider.
 */
class WidgetFileDecorationProvider : public AbstractFileDecorationProvider {
public:
  virtual QVariant headerDecoration() const override {
    // From QFileSystemModel:
    // ### TODO oh man this is ugly and doesn't even work all the way!
    // it is still 2 pixels off
    QImage pixmap(16, 1, QImage::Format_Mono);
    pixmap.fill(0);
    pixmap.setAlphaChannel(pixmap.createAlphaMask());
    return pixmap;
  }

  virtual QVariant computerDecoration() const override {
    return m_provider.icon(QFileIconProvider::Computer);
  }

  virtual QVariant folderDecoration() const override {
    return m_provider.icon(QFileIconProvider::Folder);
  }

  virtual QVariant fileDecoration() const override {
    return m_provider.icon(QFileIconProvider::File);
  }

  virtual QVariant decoration(const QFileInfo& info) const override {
    return m_provider.icon(info);
  }

  virtual QString type(const QFileInfo& info) const override {
    return m_provider.type(info);
  }

private:
  QFileIconProvider m_provider;
};


/**
 * Get the items from a combo box.
 *
 * @param comboBox combo box
 *
 * @return item texts from combo box.
 */
QStringList getItemsFromComboBox(const QComboBox* comboBox)
{
  QStringList lst;
  const int numItems = comboBox->count();
  lst.reserve(numItems);
  for (int i = 0; i < numItems; ++i) {
    lst += comboBox->itemText(i);
  }
  return lst;
}

/**
 * Set items in combo box and add current item if not already existing.
 * @param items combo box item
 * @param currentItem current item
 * @param comboBox combo box to set
 */
void setItemsInComboBox(const QStringList& items, const QString& currentItem,
                        QComboBox* comboBox)
{
  QStringList allItems = items;
  int idx = allItems.indexOf(currentItem);
  if (idx == -1) {
    allItems.append(currentItem);
    idx = allItems.size() - 1;
  }
  // Block signals on combo box while setting contents to avoid
  // editTextChanged() signals causing configuration changes.
  comboBox->blockSignals(true);
  if (!allItems.isEmpty()) {
    comboBox->clear();
    comboBox->addItems(allItems);
  }
  comboBox->setCurrentIndex(idx);
  comboBox->blockSignals(false);
}

}

/**
 * Constructs an Id3Form as a child of 'parent', with the
 * name 'name' and widget flags set to 'f'.
 * @param app application
 * @param parent parent widget
 */
Kid3Form::Kid3Form(Kid3Application* app, BaseMainWindowImpl* mainWin,
                   QWidget* parent)
  : QSplitter(parent), m_pictureLabel(nullptr), m_app(app), m_mainWin(mainWin),
    m_iconProvider(new WidgetFileDecorationProvider)
{
  setObjectName(QLatin1String("Kid3Form"));

  FOR_ALL_TAGS(tagNr) {
    m_tagContext[tagNr] = new Kid3FormTagContext(this, tagNr);
    if (tagNr != Frame::Tag_Id3v1) {
      m_app->getFrameList(tagNr)->setFrameEditor(m_mainWin);
    }
  }

  if (!s_collapsePixmap) {
    s_collapsePixmap = new QPixmap(const_cast<const char**>(collapse_xpm));
  }
  if (!s_expandPixmap) {
    s_expandPixmap = new QPixmap(const_cast<const char**>(expand_xpm));
  }

  setAcceptDrops(true);
  setWindowTitle(tr("Kid3"));

  m_leftSideWidget = new QStackedWidget(this);
  m_vSplitter = new QSplitter(Qt::Vertical);
  m_leftSideWidget->addWidget(m_vSplitter);
  m_fileListBox = new FileList(m_vSplitter, m_mainWin);
  FileProxyModel* fileProxyModel = m_app->getFileProxyModel();
  if (FileSystemModel* fsModel =
          qobject_cast<FileSystemModel*>(fileProxyModel->sourceModel())) {
    fsModel->setDecorationProvider(m_iconProvider.data());
  }
  CoreTaggedFileIconProvider* tagIconProvider = fileProxyModel->getIconProvider();
  tagIconProvider->setModifiedIcon(
        QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon));
  int iconHeight = (((fontMetrics().height() - 1) / 16) + 1) * 16;
  tagIconProvider->setRequestedSize(QSize(iconHeight, iconHeight));
  m_fileListBox->setModel(fileProxyModel);
  m_fileListBox->setSelectionModel(m_app->getFileSelectionModel());
  m_dirListBox = new ConfigurableTreeView(m_vSplitter);
  m_dirListBox->setObjectName(QLatin1String("DirList"));
  m_dirListBox->setItemsExpandable(false);
  m_dirListBox->setRootIsDecorated(false);
  m_dirListBox->setModel(m_app->getDirProxyModel());
  m_dirListBox->setSelectionModel(m_app->getDirSelectionModel());
  connect(m_dirListBox, &QAbstractItemView::activated, this,
      &Kid3Form::dirSelected);

  connect(m_app, &Kid3Application::fileRootIndexChanged,
          this, &Kid3Form::setFileRootIndex);
  connect(m_app, &Kid3Application::dirRootIndexChanged,
          this, &Kid3Form::setDirRootIndex);
  connect(m_app, &Kid3Application::directoryOpened,
          this, &Kid3Form::onFirstDirectoryOpened);

  m_rightHalfVBox = new QWidget;
  auto scrollView = new QScrollArea(this);
  scrollView->setWidget(m_rightHalfVBox);
  scrollView->setWidgetResizable(true);
  auto rightHalfLayout = new QVBoxLayout(m_rightHalfVBox);
  rightHalfLayout->setSpacing(0);

  m_fileButton = new QToolButton(m_rightHalfVBox);
  m_fileButton->setIcon(*s_collapsePixmap);
  m_fileButton->setAutoRaise(true);
#ifdef Q_OS_MAC
  m_fileButton->setStyleSheet(QLatin1String("border: 0;"));
#endif
  connect(m_fileButton, &QAbstractButton::clicked, this, &Kid3Form::showHideFile);
  m_fileLabel = new QLabel(tr("F&ile"), m_rightHalfVBox);
  auto fileButtonLayout = new QHBoxLayout;
  fileButtonLayout->addWidget(m_fileButton);
  fileButtonLayout->addWidget(m_fileLabel);
  rightHalfLayout->addLayout(fileButtonLayout);

  m_fileWidget = new QWidget(m_rightHalfVBox);
  m_fileWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  rightHalfLayout->addWidget(m_fileWidget);
  auto fileLayout = new QGridLayout(m_fileWidget);

  m_nameLabel = new QLabel(tr("Name:"), m_fileWidget);
  fileLayout->addWidget(m_nameLabel, 0, 0);

  m_nameLineEdit = new QLineEdit(m_fileWidget);
  connect(m_nameLineEdit, &QLineEdit::textEdited, this,
      &Kid3Form::nameLineEditChanged);
  fileLayout->addWidget(m_nameLineEdit, 0, 1, 1, 4);
  m_fileLabel->setBuddy(m_nameLineEdit);

  QLabel* formatLabel = new QLabel(tr("Format:") + QChar(0x2191),
                                   m_fileWidget);
  fileLayout->addWidget(formatLabel, 1, 0);

  m_formatComboBox = new QComboBox(m_fileWidget);
  m_formatComboBox->setEditable(true);
  m_formatComboBox->setSizeAdjustPolicy(
        QComboBox::AdjustToMinimumContentsLengthWithIcon);
  m_formatComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_formatComboBox->setToolTip(TrackDataFormatReplacer::getToolTip());
  connect(m_formatComboBox, &QComboBox::editTextChanged,
          this, &Kid3Form::onFormatEditTextChanged);
  m_formatFromFilenameComboBox = new QComboBox(m_fileWidget);
  m_formatFromFilenameComboBox->setEditable(true);
  m_formatFromFilenameComboBox->setSizeAdjustPolicy(
        QComboBox::AdjustToMinimumContentsLengthWithIcon);
  m_formatFromFilenameComboBox->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_formatFromFilenameComboBox->setToolTip(FrameFormatReplacer::getToolTip());
  connect(m_formatFromFilenameComboBox, &QComboBox::editTextChanged,
          this, &Kid3Form::onFormatFromFilenameEditTextChanged);

  fileLayout->addWidget(m_formatComboBox, 1, 1);

  setTabOrder(m_fileListBox, m_dirListBox);
  setTabOrder(m_dirListBox, m_nameLineEdit);
  setTabOrder(m_nameLineEdit, m_formatComboBox);
  setTabOrder(m_formatComboBox, m_formatFromFilenameComboBox);

  QWidget* tabWidget = m_formatFromFilenameComboBox;

  QLabel* fromTagLabel = new QLabel(tr("From:"), m_fileWidget);
  fileLayout->addWidget(fromTagLabel, 1, 2);
  int column = 3;
  FOR_ALL_TAGS(tagNr) {
    if (tagNr <= Frame::Tag_2) {
      QString tagStr = Frame::tagNumberToString(tagNr);
      m_fnButton[tagNr] = new QPushButton(tr("Tag %1").arg(tagStr),
                                          m_fileWidget);
      m_fnButton[tagNr]->setToolTip(tr("Filename from Tag %1").arg(tagStr));
      connect(m_fnButton[tagNr], &QAbstractButton::clicked,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::getFilenameFromTags);
      fileLayout->addWidget(m_fnButton[tagNr], 1, column++);
      setTabOrder(tabWidget, m_fnButton[tagNr]);
      tabWidget = m_fnButton[tagNr];
    } else {
      m_fnButton[tagNr] = nullptr;
    }
  }

  QLabel* formatFromFilenameLabel = new QLabel(tr("Format:") + QChar(0x2193),
                                               m_fileWidget);
  fileLayout->addWidget(formatFromFilenameLabel, 2, 0);

  fileLayout->addWidget(m_formatFromFilenameComboBox, 2, 1);

  QLabel* toTagLabel = new QLabel(tr("To:"), m_fileWidget);
  fileLayout->addWidget(toTagLabel, 2, 2);
  column = 3;
  FOR_ALL_TAGS(tagNr) {
    if (tagNr <= Frame::Tag_2) {
      QString tagStr = Frame::tagNumberToString(tagNr);
      m_toTagButton[tagNr] =
        new QPushButton(tr("Tag %1").arg(tagStr), m_fileWidget);
      m_toTagButton[tagNr]->setToolTip(tr("Tag %1 from Filename").arg(tagStr));
      connect(m_toTagButton[tagNr], &QAbstractButton::clicked,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::getTagsFromFilename);
      fileLayout->addWidget(m_toTagButton[tagNr], 2, column++);
      setTabOrder(tabWidget, m_toTagButton[tagNr]);
      tabWidget = m_toTagButton[tagNr];
    } else {
      m_toTagButton[tagNr] = nullptr;
    }
  }

  FOR_ALL_TAGS(tagNr) {
    m_tagButton[tagNr] = new QToolButton(m_rightHalfVBox);
    m_tagButton[tagNr]->setIcon(*s_collapsePixmap);
    m_tagButton[tagNr]->setAutoRaise(true);
#ifdef Q_OS_MAC
    m_tagButton[tagNr]->setStyleSheet(QLatin1String("border: 0;"));
#endif
    connect(m_tagButton[tagNr], &QAbstractButton::clicked,
            tag(tagNr), &Kid3FormTagContext::showHideTag);
    m_tagLabel[tagNr] = new QLabel(
          tr("Tag &%1").arg(Frame::tagNumberToString(tagNr)), m_rightHalfVBox);
    auto tagButtonLayout = new QHBoxLayout;
    tagButtonLayout->addWidget(m_tagButton[tagNr]);
    tagButtonLayout->addWidget(m_tagLabel[tagNr]);
    rightHalfLayout->addLayout(tagButtonLayout);

    m_tagWidget[tagNr] = new QWidget(m_rightHalfVBox);
    if (tagNr == Frame::Tag_Id3v1) {
      m_tagWidget[tagNr]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    rightHalfLayout->addWidget(m_tagWidget[tagNr], 100);

    auto idHBoxLayout = new QHBoxLayout(m_tagWidget[tagNr]);
    m_frameTable[tagNr] = new FrameTable(
          m_app->frameModel(tagNr), m_app->genreModel(tagNr), m_tagWidget[tagNr]);
    m_frameTable[tagNr]->setSelectionModel(m_app->getFramesSelectionModel(tagNr));
    idHBoxLayout->addWidget(m_frameTable[tagNr], tagNr == Frame::Tag_Id3v1 ? 100 : 0);
    m_tagLabel[tagNr]->setBuddy(m_frameTable[tagNr]);

    auto buttonsVBoxLayout = new QVBoxLayout;
    idHBoxLayout->addLayout(buttonsVBoxLayout);

    if (tagNr <= Frame::Tag_2) {
      Frame::TagNumber otherTagNr = tagNr == Frame::Tag_1
          ? Frame::Tag_2
          : tagNr == Frame::Tag_2 ? Frame::Tag_1 : Frame::Tag_NumValues;
      m_id3PushButton[tagNr] =
        new QPushButton(tr("From Tag %1")
                        .arg(Frame::tagNumberToString(otherTagNr)),
                        m_tagWidget[tagNr]);
      connect(m_id3PushButton[tagNr], &QAbstractButton::clicked,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::copyToOtherTag);
      buttonsVBoxLayout->addWidget(m_id3PushButton[tagNr]);
      setTabOrder(tabWidget, m_id3PushButton[tagNr]);

      QPushButton* copyPushButton = new QPushButton(tr("Copy"),
                                                    m_tagWidget[tagNr]);
      connect(copyPushButton, &QAbstractButton::clicked,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::copyTags);
      buttonsVBoxLayout->addWidget(copyPushButton);
      setTabOrder(m_id3PushButton[tagNr], copyPushButton);

      QPushButton* pastePushButton =
        new QPushButton(tr("Paste"), m_tagWidget[tagNr]);
      connect(pastePushButton, &QAbstractButton::clicked,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::pasteTags);
      buttonsVBoxLayout->addWidget(pastePushButton);
      setTabOrder(copyPushButton, pastePushButton);
      tabWidget = pastePushButton;
    } else {
      m_id3PushButton[tagNr] = new QPushButton(tr("From"));
      auto menu = new QMenu(this);
      QAction* action = menu->addAction(tr("Filename"));
      connect(action, &QAction::triggered,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::getTagsFromFilename);
      FOR_ALL_TAGS(fromTagNr) {
        if (fromTagNr != tagNr) {
          action = menu->addAction(
            tr("Tag %1").arg(Frame::tagNumberToString(fromTagNr)));
          QByteArray ba;
          ba.append(static_cast<char>(fromTagNr));
          ba.append(static_cast<char>(tagNr));
          action->setData(ba);
          connect(action, &QAction::triggered,
                  this, &Kid3Form::copyTagsActionData);
        }
      }
      action = menu->addAction(tr("Paste"));
      connect(action, &QAction::triggered,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::pasteTags);
      m_id3PushButton[tagNr]->setMenu(menu);
      buttonsVBoxLayout->addWidget(m_id3PushButton[tagNr]);
      setTabOrder(tabWidget, m_id3PushButton[tagNr]);

      QPushButton* toButton = new QPushButton(tr("To"));
      menu = new QMenu(this);
      action = menu->addAction(tr("Filename"));
      connect(action, &QAction::triggered,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::getFilenameFromTags);
      FOR_ALL_TAGS(fromTagNr) {
        if (fromTagNr != tagNr) {
          action = menu->addAction(
            tr("Tag %1").arg(Frame::tagNumberToString(fromTagNr)));
          QByteArray ba;
          ba.append(static_cast<char>(tagNr));
          ba.append(static_cast<char>(fromTagNr));
          action->setData(ba);
          connect(action, &QAction::triggered,
                  this, &Kid3Form::copyTagsActionData);
        }
      }
      action = menu->addAction(tr("Copy"));
      connect(action, &QAction::triggered,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::copyTags);
      toButton->setMenu(menu);
      buttonsVBoxLayout->addWidget(toButton);
      setTabOrder(m_id3PushButton[tagNr], toButton);
      tabWidget = toButton;
    }

    QPushButton* removePushButton =
      new QPushButton(tr("Remove"), m_tagWidget[tagNr]);
    connect(removePushButton, &QAbstractButton::clicked,
            m_app->tag(tagNr), &Kid3ApplicationTagContext::removeTags);
    buttonsVBoxLayout->addWidget(removePushButton);
    setTabOrder(tabWidget, removePushButton);
    tabWidget = removePushButton;

    if (tagNr != Frame::Tag_Id3v1) {
      QFrame* frameLine = new QFrame;
      frameLine->setFrameShape(QFrame::HLine);
      frameLine->setFrameShadow(QFrame::Sunken);
      buttonsVBoxLayout->addWidget(frameLine);

      QPushButton* editFramesPushButton =
        new QPushButton(tr("Edit..."), m_tagWidget[tagNr]);
      connect(editFramesPushButton, &QAbstractButton::clicked,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::editFrame);
      buttonsVBoxLayout->addWidget(editFramesPushButton);
      setTabOrder(tabWidget, editFramesPushButton);
      QPushButton* framesAddPushButton =
        new QPushButton(tr("Add..."), m_tagWidget[tagNr]);
      connect(framesAddPushButton, &QAbstractButton::clicked,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::addFrame);
      buttonsVBoxLayout->addWidget(framesAddPushButton);
      setTabOrder(editFramesPushButton, framesAddPushButton);
      QPushButton* deleteFramesPushButton =
        new QPushButton(tr("Delete"), m_tagWidget[tagNr]);
      connect(deleteFramesPushButton, &QAbstractButton::clicked,
              m_app->tag(tagNr), &Kid3ApplicationTagContext::deleteFrame);
      buttonsVBoxLayout->addWidget(deleteFramesPushButton);
      setTabOrder(framesAddPushButton, deleteFramesPushButton);
      tabWidget = deleteFramesPushButton;
    }
    if (tagNr == Frame::Tag_Picture) {
      m_pictureLabel = new PictureLabel(this);
      m_pictureLabel->installEventFilter(new PictureDblClickHandler(m_app));
      buttonsVBoxLayout->addWidget(m_pictureLabel);
    }

    buttonsVBoxLayout->addItem(
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
  }

  rightHalfLayout->insertStretch(-1);

  FOR_ALL_TAGS(tagNr) {
    setTabOrder(tabWidget, m_frameTable[tagNr]);
    tabWidget = m_frameTable[tagNr];
  }
}

/**
 * Destructor.
 */
Kid3Form::~Kid3Form()
{
  m_app->removeFrameEditor(m_mainWin);
}

/**
 * Handle event when mouse is moved while dragging.
 *
 * @param ev drag event.
 */
void Kid3Form::dragMoveEvent(QDragMoveEvent* ev)
{
  if (ev->mimeData()->hasFormat(QLatin1String("text/uri-list")) ||
      ev->mimeData()->hasImage()) {
    ev->acceptProposedAction();
  } else {
    ev->ignore();
  }
}

/**
 * Accept drag.
 *
 * @param ev drag event.
 */
void Kid3Form::dragEnterEvent(QDragEnterEvent* ev)
{
  Kid3Form::dragMoveEvent(ev);
}

/**
 * Handle event when mouse leaves widget while dragging.
 *
 * @param ev drag event.
 */
void Kid3Form::dragLeaveEvent(QDragLeaveEvent* ev)
{
  ev->accept();
}

/**
 * Handle drop event.
 *
 * @param ev drop event.
 */
void Kid3Form::dropEvent(QDropEvent* ev)
{
  if (ev->mimeData()->hasImage()) {
    QImage image = qvariant_cast<QImage>(ev->mimeData()->imageData());
    ev->acceptProposedAction();
    m_app->dropImage(image);
    return;
  } else if (ev->mimeData()->hasFormat(QLatin1String("text/uri-list"))) {
    QList<QUrl> urls = ev->mimeData()->urls();
    ev->acceptProposedAction();
    m_app->dropUrls(urls, ev->source() != nullptr);
  } else {
    ev->ignore();
  }
}

/**
 * Filename line edit is changed.
 * @param txt contents of line edit
 */
void Kid3Form::nameLineEditChanged(const QString& txt)
{
  formatLineEdit(m_nameLineEdit, txt, &FilenameFormatConfig::instance());
}

/**
 * Mark the filename as changed.
 * @param en true to mark as changed
 */
void Kid3Form::markChangedFilename(bool en)
{
  if (en) {
    QPalette changedPalette(m_nameLabel->palette());
    changedPalette.setBrush(QPalette::Active, QPalette::Window,
                            changedPalette.mid());
    m_nameLabel->setPalette(changedPalette);
  } else {
    m_nameLabel->setPalette(QPalette());
  }
  m_nameLabel->setAutoFillBackground(en);
}

/**
 * Format string within line edit.
 *
 * @param le   line edit
 * @param txt  text in line edit
 * @param fcfg format configuration
 */
void Kid3Form::formatLineEdit(QLineEdit* le, const QString& txt,
               const FormatConfig* fcfg)
{
  if (fcfg->formatWhileEditing()) {
    QString str(txt);
    fcfg->formatString(str);
    if (str != txt) {
      int curPos = le->cursorPosition();
      le->setText(str);
      le->setCursorPosition(curPos + str.length() - txt.length());
    }
  }
}

/**
 * Directory list box directory selected.
 *
 * @param index selected item
 */
void Kid3Form::dirSelected(const QModelIndex& index)
{
  QString dirPath = index.data(FileSystemModel::FilePathRole).toString();
  if (!dirPath.isEmpty()) {
    m_app->setDirUpIndex(
        dirPath.endsWith(QLatin1String("..")) ? index.parent() : QModelIndex());
    m_mainWin->updateCurrentSelection();
    m_mainWin->confirmedOpenDirectory({dirPath});
  }
}

/**
 * Enable or disable controls requiring tags.
 * @param tagNr tag number
 * @param enable true to enable
 */
void Kid3Form::enableControls(Frame::TagNumber tagNr, bool enable)
{
  if (m_fnButton[tagNr]) {
    m_fnButton[tagNr]->setEnabled(enable);
  }
  if (m_toTagButton[tagNr]) {
    m_toTagButton[tagNr]->setEnabled(enable);
  }
  Frame::TagNumber otherTagNr = tagNr == Frame::Tag_1
      ? Frame::Tag_2
      : tagNr == Frame::Tag_2 ? Frame::Tag_1 : Frame::Tag_NumValues;
  if (otherTagNr < Frame::Tag_NumValues) {
    m_id3PushButton[otherTagNr]->setEnabled(enable);
  }
  m_tagWidget[tagNr]->setEnabled(enable);
  if (tagNr > Frame::Tag_2) {
    m_tagButton[tagNr]->setVisible(enable);
    m_tagLabel[tagNr]->setVisible(enable);
  }
}

/**
 * Display the tag format.
 * @param tagNr tag number
 * @param str string describing format, e.g. "ID3v1.1"
 */
void Kid3Form::setTagFormat(Frame::TagNumber tagNr, const QString& str)
{
  QString txt = tr("Tag &%1").arg(Frame::tagNumberToString(tagNr));
  if (!str.isEmpty()) {
    txt += QLatin1String(": ");
    txt += str;
  }
  m_tagLabel[tagNr]->setText(txt);
}

/**
 * Adjust the size of the right half box.
 */
void Kid3Form::adjustRightHalfBoxSize()
{
  m_rightHalfVBox->adjustSize();
}

/**
 * Hide or show file controls.
 *
 * @param hide true to hide, false to show
 */
void Kid3Form::hideFile(bool hide)
{
  if (hide) {
    m_fileWidget->hide();
    m_fileButton->setIcon(*s_expandPixmap);
  } else {
    m_fileWidget->show();
    m_fileButton->setIcon(*s_collapsePixmap);
  }
}

/**
 * Hide or show tag controls.
 * @param tagNr tag number
 * @param hide true to hide, false to show
 */
void Kid3Form::hideTag(Frame::TagNumber tagNr, bool hide)
{
  if (hide) {
    m_tagWidget[tagNr]->hide();
    m_tagButton[tagNr]->setIcon(*s_expandPixmap);
  } else {
    m_tagWidget[tagNr]->show();
    m_tagButton[tagNr]->setIcon(*s_collapsePixmap);
  }
}

/**
 * Toggle visibility of file controls.
 */
void Kid3Form::showHideFile()
{
  hideFile(!m_fileWidget->isHidden());
}

/**
 * Toggle visibility of tag controls.
 * @param tagNr tag number
 */
void Kid3Form::showHideTag(Frame::TagNumber tagNr)
{
  hideTag(tagNr, !m_tagWidget[tagNr]->isHidden());
}

/**
 * Set format text configuration when format edit text is changed.
 * @param text format text
 */
void Kid3Form::onFormatEditTextChanged(const QString& text)
{
  FileConfig::instance().setToFilenameFormat(text);
}

/**
 * Set format from filename text configuration when edit text is changed.
 * @param text format text
 */
void Kid3Form::onFormatFromFilenameEditTextChanged(const QString& text)
{
  FileConfig::instance().setFromFilenameFormat(text);
}

/**
 * Update sorting after directory is opened for the first time.
 * The sort order of the file list is not correct if it is not explicitly
 * sorted the first time.
 */
void Kid3Form::onFirstDirectoryOpened()
{
  // Only call this once.
  disconnect(m_app, &Kid3Application::directoryOpened,
             this, &Kid3Form::onFirstDirectoryOpened);
  const GuiConfig& guiCfg = GuiConfig::instance();
  m_app->getFileProxyModel()->sort(guiCfg.fileListSortColumn(),
                                   guiCfg.fileListSortOrder());
  int firstFileSectionSize = 0;
  const QHeaderView* fileHeader = m_fileListBox->header();
  const auto columns = guiCfg.fileListVisibleColumns();
  for (int column : columns) {
    m_fileListBox->resizeColumnToContents(column);
    if (firstFileSectionSize <= 0 && fileHeader) {
      firstFileSectionSize = fileHeader->sectionSize(column);
    }
  }
  m_fileListBox->scrollTo(m_fileListBox->currentIndex());

  int firstDirSectionSize = 0;
  QHeaderView* dirHeader = m_dirListBox->header();
  const auto dirColumns = guiCfg.dirListVisibleColumns();
  for (int column : dirColumns) {
    m_dirListBox->resizeColumnToContents(column);
    if (firstDirSectionSize <= 0 && dirHeader) {
      firstDirSectionSize = dirHeader->sectionSize(column);
      if (firstDirSectionSize < firstFileSectionSize) {
        // The directory column often only contains "." and "..", which results
        // in a small size. Make it at least as wide as the corresponding
        // file list column.
        dirHeader->resizeSection(column, firstFileSectionSize);
      }
    }
  }
}

/**
 * Hide or show picture.
 *
 * @param hide true to hide, false to show
 */
void Kid3Form::hidePicture(bool hide)
{
  if (!m_pictureLabel)
    return;

  if (hide) {
    m_pictureLabel->hide();
  } else {
    m_pictureLabel->show();
  }
}

/**
 * Set focus on filename controls.
 */
void Kid3Form::setFocusFilename()
{
  m_nameLineEdit->setFocus();
}

/**
 * Set focus on tag controls.
 * @param tagNr tag number
 */
void Kid3Form::setFocusTag(Frame::TagNumber tagNr)
{
  m_frameTable[tagNr]->setFocus();
}

/**
 * Set focus on file list.
 */
void Kid3Form::setFocusFileList()
{
  m_fileListBox->setFocus();
}

/**
 * Set focus on directory list.
 */
void Kid3Form::setFocusDirList()
{
  m_dirListBox->setFocus();
}

/**
 * Save the local settings to the configuration.
 */
void Kid3Form::saveConfig()
{
  GuiConfig& guiCfg = GuiConfig::instance();
  FileConfig& fileCfg = FileConfig::instance();
  guiCfg.setSplitterSizes(sizes());
  guiCfg.setVSplitterSizes(m_vSplitter->sizes());
  fileCfg.setToFilenameFormat(m_formatComboBox->currentText());
  fileCfg.setToFilenameFormats(getItemsFromComboBox(m_formatComboBox));
  fileCfg.setFromFilenameFormat(m_formatFromFilenameComboBox->currentText());
  fileCfg.setFromFilenameFormats(getItemsFromComboBox(m_formatFromFilenameComboBox));
  if (!guiCfg.autoHideTags()) {
    guiCfg.setHideFile(m_fileWidget->isHidden());
    FOR_ALL_TAGS(tagNr) {
      guiCfg.setHideTag(tagNr, m_tagWidget[tagNr]->isHidden());
    }
  }
  int column;
  Qt::SortOrder order;
  m_fileListBox->getSortByColumn(column, order);
  guiCfg.setFileListSortColumn(column);
  guiCfg.setFileListSortOrder(order);
  guiCfg.setFileListVisibleColumns(m_fileListBox->getVisibleColumns());
  m_dirListBox->getSortByColumn(column, order);
  guiCfg.setDirListSortColumn(column);
  guiCfg.setDirListSortOrder(order);
  guiCfg.setDirListVisibleColumns(m_dirListBox->getVisibleColumns());
}

/**
 * Read the local settings from the configuration.
 */
void Kid3Form::readConfig()
{
  const GuiConfig& guiCfg = GuiConfig::instance();
  const FileConfig& fileCfg = FileConfig::instance();
  if (!guiCfg.splitterSizes().isEmpty()) {
    setSizes(guiCfg.splitterSizes());
  } else {
    setSizes({307, 601});
  }
  if (!guiCfg.vSplitterSizes().isEmpty()) {
    m_vSplitter->setSizes(guiCfg.vSplitterSizes());
  } else {
    m_vSplitter->setSizes({451, 109});
  }

  setToFilenameFormats();
  setFromFilenameFormats();
  connect(&fileCfg, &FileConfig::toFilenameFormatsChanged,
          this, &Kid3Form::setToFilenameFormats, Qt::UniqueConnection);
  connect(&fileCfg, &FileConfig::fromFilenameFormatsChanged,
          this, &Kid3Form::setFromFilenameFormats, Qt::UniqueConnection);

  if (!guiCfg.autoHideTags()) {
    hideFile(guiCfg.hideFile());
    FOR_ALL_TAGS(tagNr) {
      hideTag(tagNr, guiCfg.hideTag(tagNr));
    }
  }
  hidePicture(guiCfg.hidePicture());
  m_fileListBox->sortByColumn(guiCfg.fileListSortColumn(),
                              guiCfg.fileListSortOrder());
  m_fileListBox->setVisibleColumns(guiCfg.fileListVisibleColumns());
  m_dirListBox->sortByColumn(guiCfg.dirListSortColumn(),
                             guiCfg.dirListSortOrder());
  m_dirListBox->setVisibleColumns(guiCfg.dirListVisibleColumns());
}

/**
 * Set items of "Format <arrow up>" combo box from file configuration.
 */
void Kid3Form::setToFilenameFormats()
{
  const FileConfig& fileCfg = FileConfig::instance();
  setItemsInComboBox(fileCfg.toFilenameFormats(), fileCfg.toFilenameFormat(),
                     m_formatComboBox);
}

/**
 * Set items of "Format <arrow down>" combo box from file configuration.
 */
void Kid3Form::setFromFilenameFormats()
{
  const FileConfig& fileCfg = FileConfig::instance();
  setItemsInComboBox(fileCfg.fromFilenameFormats(), fileCfg.fromFilenameFormat(),
                     m_formatFromFilenameComboBox);
}

/**
 * Set preview picture data.
 * @param data picture data, empty if no picture is available
 */
void Kid3Form::setPictureData(const QByteArray& data)
{
  if (m_pictureLabel) {
    m_pictureLabel->setData(data);
  }
}

/**
 * Set details info text.
 *
 * @param str detail information summary as string
 */
void Kid3Form::setDetailInfo(const QString& str)
{
  m_fileLabel->setText(!str.isEmpty()
                       ? tr("F&ile") + QLatin1String(": ") + str
                       : tr("F&ile"));
}

/**
 * Select all files.
 */
void Kid3Form::selectAllFiles()
{
  m_fileListBox->selectAll();
}

/**
 * Deselect all files.
 */
void Kid3Form::deselectAllFiles()
{
  m_fileListBox->clearSelection();
}

/**
 * Set the next file as the current file.
 *
 * @param select true to select the file
 * @param onlyTaggedFiles only consider tagged files
 *
 * @return true if a next file exists.
 */
bool Kid3Form::nextFile(bool select, bool onlyTaggedFiles)
{
  FrameTable* editingFrameTable = getEditingFrameTable();
  bool ok = m_app->nextFile(select, onlyTaggedFiles);
  if (ok && editingFrameTable) {
    editingFrameTable->edit(editingFrameTable->currentIndex());
  }
  return ok;
}

/**
 * Set the previous file as the current file.
 *
 * @param select true to select the file
 * @param onlyTaggedFiles only consider tagged files
 *
 * @return true if a previous file exists.
 */
bool Kid3Form::previousFile(bool select, bool onlyTaggedFiles)
{
  FrameTable* editingFrameTable = getEditingFrameTable();
  bool ok = m_app->previousFile(select, onlyTaggedFiles);
  if (ok && editingFrameTable) {
    editingFrameTable->edit(editingFrameTable->currentIndex());
  }
  return ok;
}

/**
 * Select the next tagged file as the current file.
 * Same as nextFile() with default arguments, provided for functor-based
 * connections.
 * @return true if a next file exists.
 */
bool Kid3Form::selectNextTaggedFile()
{
  return nextFile(true, true);
}

/**
 * Select the previous tagged file as the current file.
 * Same as previousFile() with default arguments, provided for functor-based
 * connections.
 * @return true if a previous file exists.
 */
bool Kid3Form::selectPreviousTaggedFile()
{
  return previousFile(true, true);
}

/**
 * Get frame table which is currently in editing state.
 * The returned frame table can be used to restore the editing state after
 * changing the current file.
 * @return frame table which is in editing state, 0 if none.
 */
FrameTable* Kid3Form::getEditingFrameTable() const
{
  if (QWidget* focusWidget = QApplication::focusWidget()) {
    FOR_ALL_TAGS(tagNr) {
      if (m_frameTable[tagNr]->getCurrentEditor() == focusWidget) {
        return m_frameTable[tagNr];
      }
    }
  }
  return nullptr;
}

/**
 * Set the root index of the file list.
 *
 * @param index root index of directory in file proxy model
 */
void Kid3Form::setFileRootIndex(const QModelIndex& index)
{
  if (index.isValid()) {
    m_fileListBox->setRootIndex(index);
    m_fileListBox->scrollTo(m_fileListBox->currentIndex());
  }
}

/**
 * Set the root index of the directory list.
 *
 * @param index root index of directory in directory model
 */
void Kid3Form::setDirRootIndex(const QModelIndex& index)
{
  if (index.isValid()) {
    m_dirListBox->setRootIndex(index);
  }
}

/**
 * Set a widget to be displayed at the left side instead of the file lists.
 * @param widget widget to be shown at the left side
 */
void Kid3Form::setLeftSideWidget(QWidget* widget)
{
  int idx = m_leftSideWidget->addWidget(widget);
  m_leftSideWidget->setCurrentIndex(idx);
}

/**
 * Remove widget set with setLeftSideWidget().
 *
 * The widget will not be deleted.
 *
 * @param widget widget to be removed
 */
void Kid3Form::removeLeftSideWidget(QWidget* widget)
{
  m_leftSideWidget->removeWidget(widget);
}

/**
 * Copy tags using QAction::data().
 * The source and destination tag numbers are taken from the first two bytes
 * in QAction::data().toByteArray() if the sender() is a QAction.
 */
void Kid3Form::copyTagsActionData()
{
  if (auto action = qobject_cast<QAction*>(sender())) {
    QByteArray ba = action->data().toByteArray();
    if (ba.size() == 2) {
      Frame::TagNumber srcTagNr = Frame::tagNumberCast(ba.at(0));
      Frame::TagNumber dstTagNr = Frame::tagNumberCast(ba.at(1));
      if (srcTagNr != Frame::Tag_NumValues &&
          dstTagNr != Frame::Tag_NumValues) {
        m_app->copyTag(srcTagNr, dstTagNr);
      }
    }
  }
}
