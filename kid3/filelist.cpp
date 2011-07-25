/**
 * \file filelist.cpp
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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
#include "kid3mainwindow.h"
#include "configstore.h"
#include "externalprocess.h"
#include "qtcompatmac.h"

namespace {

/**
 * Replaces context command format codes in a string.
 */
class CommandFormatReplacer : public FrameFormatReplacer {
public:
  /**
   * Constructor.
   *
   * @param frames frame collection
   * @param str    string with format codes
   * @param files  file list
   * @param isDir  true if directory
   */
  explicit CommandFormatReplacer(
    const FrameCollection& frames, const QString& str,
    const QStringList& files, bool isDir);

  /**
   * Destructor.
   */
  virtual ~CommandFormatReplacer();

  /**
   * Get help text for supported format codes.
   *
   * @param onlyRows if true only the tr elements are returned,
   *                 not the surrounding table
   *
   * @return help text.
   */
  static QString getToolTip(bool onlyRows = false);

protected:
  /**
   * Replace a format code (one character %c or multiple characters %{chars}).
   * Supported format fields:
   * Those supported by FrameFormatReplacer::getReplacement()
   * %f %{file} filename
   * %d %{directory} directory name
   * %b %{browser} the web browser set in the configuration
   *
   * @param code format code
   *
   * @return replacement string,
   *         QString::null if code not found.
   */
  virtual QString getReplacement(const QString& code) const;

private:
  const QStringList& m_files;
  const bool m_isDir;
};


/**
 * Constructor.
 *
 * @param frames frame collection
 * @param str    string with format codes
 * @param files  file list
 * @param isDir  true if directory
 */
CommandFormatReplacer::CommandFormatReplacer(
  const FrameCollection& frames, const QString& str,
  const QStringList& files, bool isDir) :
  FrameFormatReplacer(frames, str), m_files(files), m_isDir(isDir) {}

/**
 * Destructor.
 */
CommandFormatReplacer::~CommandFormatReplacer() {}

/**
 * Replace a format code (one character %c or multiple characters %{chars}).
 * Supported format fields:
 * Those supported by FrameFormatReplacer::getReplacement()
 * %f %{file} filename
 * %d %{directory} directory name
 * %b %{browser} the web browser set in the configuration
 *
 * @param code format code
 *
 * @return replacement string,
 *         QString::null if code not found.
 */
QString CommandFormatReplacer::getReplacement(const QString& code) const
{
  QString result = FrameFormatReplacer::getReplacement(code);
  if (result.isNull()) {
    QString name;

    if (code.length() == 1) {
      static const struct {
        char shortCode;
        const char* longCode;
      } shortToLong[] = {
        { 'f', "file" },
        { 'd', "directory" },
        { 'b', "browser" }
      };
      const char c = code[0].toLatin1();
      for (unsigned i = 0; i < sizeof(shortToLong) / sizeof(shortToLong[0]); ++i) {
        if (shortToLong[i].shortCode == c) {
          name = shortToLong[i].longCode;
          break;
        }
      }
    } else if (code.length() > 1) {
      name = code;
    }

    if (!name.isNull()) {
      if (name == "file") {
        result = m_files.front();
      } else if (name == "directory") {
        result = m_files.front();
        if (!m_isDir) {
          int sepPos = result.lastIndexOf('/');
          if (sepPos < 0) {
            sepPos = result.lastIndexOf(QDir::separator());
          }
          if (sepPos >= 0) {
            result.truncate(sepPos);
          }
        }
      } else if (name == "browser") {
        result = ConfigStore::s_miscCfg.m_browser;
      } else if (name == "url") {
        if (!m_files.empty()) {
          QUrl url;
          url.setScheme("file");
          url.setPath(m_files.front());
          result = url.toString();
        }
      }
    }
  }

  return result;
}

/**
 * Get help text for supported format codes.
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString CommandFormatReplacer::getToolTip(bool onlyRows)
{
  QString str;
  if (!onlyRows) str += "<table>\n";
  str += FrameFormatReplacer::getToolTip(true);

  str += "<tr><td>%f</td><td>%{file}</td><td>";
  str += QCM_translate("Filename");
  str += "</td></tr>\n";

  str += "<tr><td>%F</td><td>%{files}</td><td>";
  str += QCM_translate(I18N_NOOP("Filenames"));
  str += "</td></tr>\n";

  str += "<tr><td>%uf</td><td>%{url}</td><td>";
  str += QCM_translate("URL");
  str += "</td></tr>\n";

  str += "<tr><td>%uF</td><td>%{urls}</td><td>";
  str += QCM_translate(I18N_NOOP("URLs"));
  str += "</td></tr>\n";

  str += "<tr><td>%d</td><td>%{directory}</td><td>";
  str += QCM_translate(I18N_NOOP("Directory name"));
  str += "</td></tr>\n";

  str += "<tr><td>%b</td><td>%{browser}</td><td>";
  str += QCM_translate("Browser");
  str += "</td></tr>\n";

  str += "<tr><td>%ua...</td><td>%u{artist}...</td><td>";
  str += QCM_translate(I18N_NOOP("Encode as URL"));
  str += "</td></tr>\n";

  if (!onlyRows) str += "</table>\n";
  return str;
}

} // anonymous namespace


/**
 * Constructor.
 * @param parent parent widget
 * @param app    application widget
 */
FileList::FileList(QWidget* parent, Kid3MainWindow* mainWin) :
  QTreeView(parent), m_process(0), m_mainWin(mainWin)
{
  setObjectName("FileList");
  setSelectionMode(ExtendedSelection);
  setSortingEnabled(false);
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(customContextMenu(const QPoint&)));
#ifdef HAVE_PHONON
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
 * @param dirIndex index of directory in filesystem model
 * @param fileIndex index of file to select in filesystem model (optional,
 * else invalid)
 *
 * @return false if name is not directory path, else true.
 */
bool FileList::readDir(const QModelIndex& dirIndex,
                        const QModelIndex& fileIndex) {
  QAbstractProxyModel* proxyModel = qobject_cast<QAbstractProxyModel*>(model());
  QModelIndex rootIndex = proxyModel ? proxyModel->mapFromSource(dirIndex) : dirIndex;
  if (rootIndex.isValid()) {
    setRootIndex(rootIndex);
    if (fileIndex.isValid()) {
      QModelIndex index = proxyModel ? proxyModel->mapFromSource(fileIndex) : fileIndex;
      if (index.isValid()) {
        setCurrentIndex(index);
      }
    } else {
      setCurrentIndex(rootIndex);
      // Make sure that this invisible root index item is not selected
      if (selectionModel())
        selectionModel()->clearSelection();
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
  if (index.isValid() && !ConfigStore::s_miscCfg.m_contextMenuCommands.empty()) {
    QMenu menu(this);
    menu.addAction(i18n("&Expand all"), this, SLOT(expandAll()));
    menu.addAction(i18n("&Collapse all"), this, SLOT(collapseAll()));
    menu.addAction(i18n("&Rename"), m_mainWin, SLOT(renameFile()));
    menu.addAction(i18n("&Delete"), m_mainWin, SLOT(deleteFile()));
#ifdef HAVE_PHONON
    menu.addAction(i18n("&Play"), m_mainWin, SLOT(slotPlayAudio()));
#endif
    int id = 0;
    for (QList<MiscConfig::MenuCommand>::const_iterator
           it = ConfigStore::s_miscCfg.m_contextMenuCommands.begin();
         it != ConfigStore::s_miscCfg.m_contextMenuCommands.end();
         ++it) {
      menu.addAction((*it).getName());
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
    if ((*it).indexOf('%') == -1) {
      fmt.push_back(*it);
    } else {
      if (*it == "%F" || *it == "%{files}") {
        // list of files
        fmt += files;
      } else if (*it == "%uF" || *it == "%{urls}") {
        // list of URLs or URL
        QUrl url;
        url.setScheme("file");
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
        str.replace("%uf", "%{url}");
        CommandFormatReplacer cfr(frames, str, files, !dirPath.isNull());
        cfr.replacePercentCodes(FrameFormatReplacer::FSF_SupportUrlEncode);
        fmt.push_back(cfr.getString());
      }
    }
  }
  return fmt;
}

/**
 * Get help text for format codes supported by formatStringList().
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString FileList::getFormatToolTip(bool onlyRows)
{
  return CommandFormatReplacer::getToolTip(onlyRows);
}

/**
 * Execute a context menu command.
 *
 * @param id command ID
 */
void FileList::executeContextCommand(int id)
{
  if (id < static_cast<int>(ConfigStore::s_miscCfg.m_contextMenuCommands.size())) {
    QStringList args;
    const MiscConfig::MenuCommand& menuCmd = ConfigStore::s_miscCfg.m_contextMenuCommands[id];
    QString cmd = menuCmd.getCommand();

    int len = cmd.length();
    int begin;
    int end = 0;
    while (end < len) {
      begin = end;
      while (begin < len && cmd[begin] == ' ') ++begin;
      if (begin >= len) break;
      if (cmd[begin] == '"') {
        ++begin;
        QString str;
        while (begin < len) {
          if (cmd[begin] == '\\' && begin + 1 < len &&
              (cmd[begin + 1] == '\\' ||
               cmd[begin + 1] == '"')) {
            ++begin;
          } else if (cmd[begin] == '"') {
            break;
          }
          str += cmd[begin];
          ++begin;
        }
        args.push_back(str);
        end = begin;
      } else {
        end = cmd.indexOf(' ', begin + 1);
        if (end == -1) end = len;
        args.push_back(cmd.mid(begin, end - begin));
      }
      ++end;
    }

    args = formatStringList(args);

    if (!m_process) {
      m_process = new ExternalProcess(this);
    }
    if (m_process) {
      m_process->launchCommand(menuCmd.getName(), args, menuCmd.mustBeConfirmed(), menuCmd.outputShown());
    }
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
    QString name = action->text().remove('&');
    int id = 0;
    for (QList<MiscConfig::MenuCommand>::const_iterator
           it = ConfigStore::s_miscCfg.m_contextMenuCommands.begin();
         it != ConfigStore::s_miscCfg.m_contextMenuCommands.end();
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

/**
 * Play item if it is a tagged file.
 *
 * @param index model index of item
 */
void FileList::playIfTaggedFile(const QModelIndex& index)
{
#ifdef HAVE_PHONON
  if (FileProxyModel::getTaggedFileOfIndex(index)) {
    m_mainWin->slotPlayAudio();
  }
#endif
}
