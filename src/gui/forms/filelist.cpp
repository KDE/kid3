/**
 * \file filelist.cpp
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2014  Urs Fleisch
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

#include "filelist.h"
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QUrl>
#include <QMenu>
#include <QHeaderView>
#include <QDesktopServices>
#include <QMouseEvent>
#include "fileproxymodel.h"
#include "modeliterator.h"
#include "taggedfile.h"
#include "basemainwindow.h"
#include "useractionsconfig.h"
#include "guiconfig.h"
#include "playlistconfig.h"
#include "externalprocess.h"
#include "commandformatreplacer.h"

namespace {

/**
 * Create a name for an action.
 * @param text user action text
 * @return name for user action.
 */
QString nameForAction(const QString& text)
{
  QString name;
  for (auto cit = text.constBegin(); cit != text.constEnd(); ++cit) {
    if (cit->toLatin1() == '\0') {
      continue;
    }
    if (cit->isLetterOrNumber()) {
      name.append(cit->toLower());
    } else if (cit->isSpace()) {
      name.append(QLatin1Char('_'));
    }
  }
  if (!name.isEmpty()) {
    name.prepend(QLatin1String("user_"));
  }
  return name;
}

}


/**
 * Constructor.
 * @param parent parent widget
 * @param mainWin main window
 */
FileList::FileList(QWidget* parent, BaseMainWindowImpl* mainWin) :
  ConfigurableTreeView(parent), m_mainWin(mainWin),
  m_renameAction(nullptr), m_deleteAction(nullptr)
{
  setObjectName(QLatin1String("FileList"));
  setSelectionMode(ExtendedSelection);
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QWidget::customContextMenuRequested,
      this, &FileList::customContextMenu);
  connect(this, &QAbstractItemView::doubleClicked,
          this, &FileList::onDoubleClicked);
}

/**
 * Destructor.
 */
FileList::~FileList()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Returns the recommended size for the widget.
 * @return recommended size.
 */
QSize FileList::sizeHint() const
{
  return QSize(fontMetrics().maxWidth() * 25,
               QTreeView::sizeHint().height());
}

/**
 * Enable dragging if the item is pressed at the left icon side.
 * @param event mouse event
 */
void FileList::mousePressEvent(QMouseEvent* event)
{
  QPoint pos = event->pos();
  if (pos.x() < 80) {
    QModelIndex idx = indexAt(pos);
    if (const auto fsModel =
        qobject_cast<const FileProxyModel*>(idx.model())) {
      if (!FileProxyModel::getTaggedFileOfIndex(idx)) {
        // The file possibly dragged is not a tagged file, e.g. an image file.
        // Make it the only draggable file in order to keep the selection of
        // tagged files while still being able to drag an image file on them.
        const_cast<FileProxyModel*>(fsModel)->setExclusiveDraggableIndex(idx);
        setSelectionMode(MultiSelection);
      } else {
        const_cast<FileProxyModel*>(fsModel)->setExclusiveDraggableIndex(
              QPersistentModelIndex());
        setSelectionMode(ExtendedSelection);
      }
    }
    setDragEnabled(true);
  } else {
    setDragEnabled(false);
    setSelectionMode(ExtendedSelection);
  }
  ConfigurableTreeView::mousePressEvent(event);
}

/**
 * Called when a drag operation is started.
 * Reimplemented to close all tagged files before being dropped to another
 * application, which would not be able to open them on Windows.
 * @param supportedActions drop actions
 */
void FileList::startDrag(Qt::DropActions supportedActions)
{
  const auto indexes = selectedIndexes();
  for (const QModelIndex& index : indexes) {
    const QAbstractItemModel* mdl = index.model();
    if (index.column() == 0 &&
        mdl && (mdl->flags(index) & Qt::ItemIsDragEnabled)) {
      if (TaggedFile* tf = FileProxyModel::getTaggedFileOfIndex(index)) {
        tf->closeFileHandle();
      }
    }
  }
  ConfigurableTreeView::startDrag(supportedActions);
}

/**
 * Init the user actions for the context menu.
 */
void FileList::initUserActions()
{
  QMap<QString, QAction*> oldUserActions;
  oldUserActions.swap(m_userActions);
  int id = 0;
  const QList<UserActionsConfig::MenuCommand> commands =
      UserActionsConfig::instance().contextMenuCommands();
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    const QString text((*it).getName());
    const QString name = nameForAction(text);
    if (!name.isEmpty() && it->getCommand() != QLatin1String("@beginmenu")) {
      QAction* action = oldUserActions.take(name);
      if (!action) {
        action = new QAction(text, this);
        connect(action, &QAction::triggered, this, &FileList::executeSenderAction);
        emit userActionAdded(name, action);
      }
      action->setData(id);
      m_userActions.insert(name, action);
    }
    ++id;
  }
  for (auto it = oldUserActions.constBegin(); it != oldUserActions.constEnd(); ++it) {
    emit userActionRemoved(it.key(), it.value());
  }
}

/**
 * Display a context menu with operations for selected files.
 *
 * @param index index of item
 * @param pos   position where context menu is drawn on screen
 */
void FileList::contextMenu(const QModelIndex& index, const QPoint& pos)
{
  if (index.isValid()) {
    QString path;
    bool isPlaylist = false;
    if (const auto model =
            qobject_cast<const FileProxyModel*>(index.model())) {
      path = model->filePath(index);
      PlaylistConfig::formatFromFileExtension(path, &isPlaylist);
    }
    QMenu menu(this);
#if QT_VERSION >= 0x050600
    menu.addAction(tr("&Expand all"), m_mainWin, &BaseMainWindowImpl::expandFileList);
    menu.addAction(tr("&Collapse all"), this, &QTreeView::collapseAll);
#else
    menu.addAction(tr("&Expand all"), m_mainWin, SLOT(expandFileList()));
    menu.addAction(tr("&Collapse all"), this, SLOT(collapseAll()));
#endif
    if (m_renameAction) {
      menu.addAction(m_renameAction);
    }
    if (m_deleteAction) {
      menu.addAction(m_deleteAction);
    }
#if QT_VERSION >= 0x050600
    menu.addAction(tr("&Play"), m_mainWin, &BaseMainWindowImpl::slotPlayAudio);
#else
    menu.addAction(tr("&Play"), m_mainWin, SLOT(slotPlayAudio()));
#endif
    if (isPlaylist) {
      QAction* editPlaylistAction = new QAction(tr("E&dit"), &menu);
      editPlaylistAction->setData(path);
      connect(editPlaylistAction, &QAction::triggered,
              this, &FileList::editPlaylist);
      menu.addAction(editPlaylistAction);
    }
#if QT_VERSION >= 0x050600
    menu.addAction(tr("&Open"), this, &FileList::openFile);
    menu.addAction(tr("Open Containing &Folder"),
                   this, &FileList::openContainingFolder);
#else
    menu.addAction(tr("&Open"), this, SLOT(openFile()));
    menu.addAction(tr("Open Containing &Folder"),
                   this, SLOT(openContainingFolder()));
#endif
    QMenu* userMenu = &menu;
    QList<UserActionsConfig::MenuCommand> commands =
        UserActionsConfig::instance().contextMenuCommands();
    for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
      const QString text((*it).getName());
      const QString name = nameForAction(text);
      if (!text.isEmpty()) {
        if (it->getCommand() == QLatin1String("@beginmenu")) {
          userMenu = userMenu->addMenu(text);
        } else if (QAction* action = m_userActions.value(name)) {
          userMenu->addAction(action);
        }
      } else if (it->getCommand() == QLatin1String("@separator")) {
        userMenu->addSeparator();
      } else if (it->getCommand() == QLatin1String("@endmenu")) {
        if (auto parentMenu = qobject_cast<QMenu*>(userMenu->parent())) {
          userMenu = parentMenu;
        }
      }
    }
    menu.setMouseTracking(true);
    menu.exec(pos);
  }
}

/**
 * Format a string list from the selected files.
 * Supported format fields:
 * Those supported by FrameFormatReplacer::getReplacement(),
 * when prefixed with u, encoded as URL
 * %f filename
 * %F list of files
 * %uf URL of single file
 * %uF list of URLs
 * %d directory name
 * %b the web browser set in the configuration
 * %q the base directory for QML files
 *
 * @todo %f and %F are full paths, which is inconsistent with the
 * export format strings but compatible with .desktop files.
 * %d is duration in export format.
 * The export codes should be changed.
 *
 * @param format format specification
 *
 * @return formatted string list.
 */
QStringList FileList::formatStringList(const QStringList& format)
{
  QStringList files;
  TaggedFile* firstSelectedFile = nullptr;
  const QModelIndexList selItems(selectionModel()
       ? selectionModel()->selectedRows() : QModelIndexList());
  for (const QModelIndex& index : selItems) {
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
      if (!firstSelectedFile) {
        firstSelectedFile = taggedFile;
      }
      files.append(taggedFile->getAbsFilename());
    }
  }

  QString dirPath;
  if (files.isEmpty() && !selItems.isEmpty()) {
    dirPath = FileProxyModel::getPathIfIndexOfDir(selItems.first());
    if (!dirPath.isNull()) {
      files.append(dirPath);
      firstSelectedFile = TaggedFileOfDirectoryIterator::first(selItems.first());
    }
  }

  FrameCollection frames;
  QStringList fmt;
  for (auto it = format.constBegin(); it != format.constEnd(); ++it) {
    if ((*it).indexOf(QLatin1Char('%')) == -1) {
      fmt.push_back(*it);
    } else {
      if (*it == QLatin1String("%F") || *it == QLatin1String("%{files}")) {
        // list of files
        fmt += files;
      } else if (*it == QLatin1String("%uF") || *it == QLatin1String("%{urls}")) {
        // list of URLs or URL
        QUrl url;
        url.setScheme(QLatin1String("file"));
        for (auto fit = files.constBegin(); fit != files.constEnd(); ++fit) {
          url.setPath(*fit);
          fmt.push_back(url.toString());
        }
      } else {
        if (firstSelectedFile) {
          // use merged tags to format string
          frames.clear();
          for (Frame::TagNumber tagNr : Frame::allTagNumbers()) {
            if (frames.empty()) {
              firstSelectedFile->getAllFrames(tagNr, frames);
            } else {
              FrameCollection frames1;
              firstSelectedFile->getAllFrames(tagNr, frames1);
              frames.merge(frames1);
            }
          }
        }
        QString str(*it);
        str.replace(QLatin1String("%uf"), QLatin1String("%{url}"));
        CommandFormatReplacer cfr(frames, str, files, !dirPath.isNull());
        cfr.replacePercentCodes(FrameFormatReplacer::FSF_SupportUrlEncode);
        fmt.push_back(cfr.getString());
      }
    }
  }
  return fmt;
}

/**
 * Execute a context menu command.
 *
 * @param id command ID
 */
void FileList::executeContextCommand(int id)
{
  if (id < static_cast<int>(UserActionsConfig::instance().contextMenuCommands().size())) {
    QStringList args;
    const UserActionsConfig::MenuCommand& menuCmd = UserActionsConfig::instance().contextMenuCommands().at(id);
    QString cmd = menuCmd.getCommand();

    int len = cmd.length();
    int end = 0;
    while (end < len) {
      int begin = end;
      while (begin < len && cmd[begin] == QLatin1Char(' ')) ++begin;
      if (begin >= len) break;
      if (cmd[begin] == QLatin1Char('"')) {
        ++begin;
        QString str;
        while (begin < len) {
          if (cmd[begin] == QLatin1Char('\\') && begin + 1 < len &&
              (cmd[begin + 1] == QLatin1Char('\\') ||
               cmd[begin + 1] == QLatin1Char('"'))) {
            ++begin;
          } else if (cmd[begin] == QLatin1Char('"')) {
            break;
          }
          str += cmd[begin];
          ++begin;
        }
        args.push_back(str);
        end = begin;
      } else {
        end = cmd.indexOf(QLatin1Char(' '), begin + 1);
        if (end == -1) end = len;
        args.push_back(cmd.mid(begin, end - begin));
      }
      ++end;
    }

    args = formatStringList(args);

    if (!m_process) {
      m_process.reset(new ExternalProcess(m_mainWin->app(), this));
    }
    m_process->launchCommand(menuCmd.getName(), args, menuCmd.mustBeConfirmed(),
                             menuCmd.outputShown());
  }
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu, 0 to use sender() action
 */
void FileList::executeAction(QAction* action)
{
  if (!action) {
    action = qobject_cast<QAction*>(sender());
  }
  if (action) {
    bool ok;
    int id = action->data().toInt(&ok);
    if (ok) {
      executeContextCommand(id);
      return;
    }

    QString name = action->text().remove(QLatin1Char('&'));
    id = 0;
    QList<UserActionsConfig::MenuCommand> commands =
        UserActionsConfig::instance().contextMenuCommands();
    for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
      if (name == (*it).getName()) {
        executeContextCommand(id);
        break;
      }
      ++id;
    }
  }
}

/**
 * Execute context menu action which sent signal.
 * Same as executeAction() with default arguments, provided for functor-based
 * connections.
 */
void FileList::executeSenderAction()
{
  executeAction(nullptr);
}

/**
 * Display a custom context menu with operations for selected files.
 *
 * @param pos  position where context menu is drawn on screen
 */
void FileList::customContextMenu(const QPoint& pos)
{
  contextMenu(currentIndex(), mapToGlobal(pos));
}

/**
 * Handle double click to file.
 *
 * @param index model index of item
 */
void FileList::onDoubleClicked(const QModelIndex& index)
{
  if (FileProxyModel::getTaggedFileOfIndex(index)) {
    if (GuiConfig::instance().playOnDoubleClick()) {
      m_mainWin->slotPlayAudio();
    }
  } else if (const auto model =
             qobject_cast<const FileProxyModel*>(index.model())) {
    QString path = model->filePath(index);
    bool isPlaylist = false;
    PlaylistConfig::formatFromFileExtension(path, &isPlaylist);
    if (isPlaylist) {
      m_mainWin->showPlaylistEditDialog(path);
    }
  }
}

/**
 * Called when "Edit" action is called from context menu.
 */
void FileList::editPlaylist()
{
  if (auto action = qobject_cast<QAction*>(sender())) {
    m_mainWin->showPlaylistEditDialog(action->data().toString());
  }
}

/**
 * Set rename action.
 * @param action rename action
 */
void FileList::setRenameAction(QAction* action)
{
  if (m_renameAction) {
    removeAction(m_renameAction);
  }
  m_renameAction = action;
  if (m_renameAction) {
    addAction(m_renameAction);
  }
}

/**
 * Set delete action.
 * @param action delete action
 */
void FileList::setDeleteAction(QAction* action)
{
  if (m_deleteAction) {
    removeAction(m_deleteAction);
  }
  m_deleteAction = action;
  if (m_deleteAction) {
    addAction(m_deleteAction);
  }
}

/**
 * Open with standard application.
 */
void FileList::openFile()
{
  if (QItemSelectionModel* selModel = selectionModel()) {
    if (const auto fsModel =
        qobject_cast<const FileProxyModel*>(selModel->model())) {
      const auto indexes = selModel->selectedRows();
      for (const QModelIndex& index : indexes) {
        QDesktopServices::openUrl(
              QUrl::fromLocalFile(fsModel->filePath(index)));
      }
    }
  }
}

/**
 * Open containing folder.
 */
void FileList::openContainingFolder()
{
  if (QItemSelectionModel* selModel = selectionModel()) {
    QModelIndexList indexes = selModel->selectedRows();
    if (!indexes.isEmpty()) {
      const FileProxyModel* fsModel;
      QModelIndex index = indexes.first().parent();
      if (index.isValid() &&
          (fsModel = qobject_cast<const FileProxyModel*>(index.model())) != nullptr &&
          fsModel->isDir(index)) {
        QDesktopServices::openUrl(
              QUrl::fromLocalFile(fsModel->filePath(index)));
      }
    }
  }
}
