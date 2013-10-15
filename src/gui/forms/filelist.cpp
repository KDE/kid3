/**
 * \file filelist.cpp
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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

#include "filelist.h"
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QUrl>
#include <QMenu>
#include <QHeaderView>
#include "fileproxymodel.h"
#include "modeliterator.h"
#include "taggedfile.h"
#include "basemainwindow.h"
#include "useractionsconfig.h"
#include "guiconfig.h"
#include "externalprocess.h"
#include "commandformatreplacer.h"

/**
 * Constructor.
 * @param parent parent widget
 * @param mainWin main window
 */
FileList::FileList(QWidget* parent, BaseMainWindowImpl* mainWin) :
  QTreeView(parent), m_process(0), m_mainWin(mainWin)
{
  setObjectName(QLatin1String("FileList"));
  setSelectionMode(ExtendedSelection);
  setSortingEnabled(false);
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(customContextMenu(const QPoint&)));
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  connect(this, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(playIfTaggedFile(QModelIndex)));
#endif
  header()->hide();
}

/**
 * Destructor.
 */
FileList::~FileList()
{
  delete m_process;
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
 * Fill the filelist with the files found in a directory.
 *
 * @param dirIndex index of directory in file proxy model
 * @param fileIndexes indexes of files to select in file proxy model
 * (optional, else empty)
 *
 * @return false if name is not directory path, else true.
 */
bool FileList::readDir(const QPersistentModelIndex& dirIndex,
                       const QList<QPersistentModelIndex>& fileIndexes) {
  if (dirIndex.isValid()) {
    setRootIndex(dirIndex);
    if (QItemSelectionModel* selModel = selectionModel()) {
      selModel->clearSelection();
      if (!fileIndexes.isEmpty()) {
        foreach (const QPersistentModelIndex& fileIndex, fileIndexes) {
          selModel->select(fileIndex, QItemSelectionModel::Select);
          scrollTo(fileIndex);
        }
        selModel->setCurrentIndex(fileIndexes.first(),
                                  QItemSelectionModel::NoUpdate);
      } else {
        setCurrentIndex(dirIndex);
        // Make sure that this invisible root index item is not selected
        selModel->clearSelection();
      }
    }
    return true;
  }
  return false;
}

/**
 * Update the stored current selection with the list of all selected items.
 */
void FileList::updateCurrentSelection()
{
  if (!selectionModel())
    return;
  m_currentSelection.clear();
  foreach (QModelIndex index, selectionModel()->selectedIndexes()) {
    m_currentSelection.append(QPersistentModelIndex(index));
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
    QMenu menu(this);
    menu.addAction(tr("&Expand all"), m_mainWin, SLOT(expandFileList()));
    menu.addAction(tr("&Collapse all"), this, SLOT(collapseAll()));
    menu.addAction(tr("&Rename"), m_mainWin, SLOT(renameFile()));
    menu.addAction(tr("&Move to Trash"), m_mainWin, SLOT(deleteFile()));
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
    menu.addAction(tr("&Play"), m_mainWin, SLOT(slotPlayAudio()));
#endif
    int id = 0;
    for (QList<UserActionsConfig::MenuCommand>::const_iterator
           it = UserActionsConfig::instance().m_contextMenuCommands.begin();
         it != UserActionsConfig::instance().m_contextMenuCommands.end();
         ++it) {
      QString name((*it).getName());
      if (!name.isEmpty()) {
        menu.addAction(name);
      }
      ++id;
    }
    connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(executeAction(QAction*)));
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
  TaggedFile* firstSelectedFile = 0;
  QModelIndexList selItems(selectionModel()
       ? selectionModel()->selectedIndexes() : QModelIndexList());
  foreach (QModelIndex index, selItems) {
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
  for (QStringList::const_iterator it = format.begin();
       it != format.end();
       ++it) {
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
        for (QStringList::const_iterator fit = files.begin();
             fit != files.end();
             ++fit) {
          url.setPath(*fit);
          fmt.push_back(url.toString());
        }
      } else {
        if (firstSelectedFile) {
          // use merged tags 1 and 2 to format string
          FrameCollection frames1;
          firstSelectedFile->getAllFramesV1(frames1);
          firstSelectedFile->getAllFramesV2(frames);
          frames.merge(frames1);
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
  if (id < static_cast<int>(UserActionsConfig::instance().m_contextMenuCommands.size())) {
    QStringList args;
    const UserActionsConfig::MenuCommand& menuCmd = UserActionsConfig::instance().m_contextMenuCommands[id];
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
      m_process = new ExternalProcess(this);
    }
    m_process->launchCommand(menuCmd.getName(), args, menuCmd.mustBeConfirmed(),
                             menuCmd.outputShown());
  }
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
void FileList::executeAction(QAction* action)
{
  if (action) {
    QString name = action->text().remove(QLatin1Char('&'));
    int id = 0;
    for (QList<UserActionsConfig::MenuCommand>::const_iterator
           it = UserActionsConfig::instance().m_contextMenuCommands.begin();
         it != UserActionsConfig::instance().m_contextMenuCommands.end();
         ++it) {
      if (name == (*it).getName()) {
        executeContextCommand(id);
        break;
      }
      ++id;
    }
  }
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

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
/**
 * Play item if it is a tagged file.
 *
 * @param index model index of item
 */
void FileList::playIfTaggedFile(const QModelIndex& index)
{
  if (GuiConfig::instance().m_playOnDoubleClick &&
      FileProxyModel::getTaggedFileOfIndex(index)) {
    m_mainWin->slotPlayAudio();
  }
}
#endif
