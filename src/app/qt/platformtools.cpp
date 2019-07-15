/**
 * \file platformtools.cpp
 * Platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Mar 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "platformtools.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QIcon>
#include <QSettings>
#include <QCoreApplication>
#include "config.h"
#include "browserdialog.h"
#include "messagedialog.h"
#include "kid3settings.h"
#include "mainwindowconfig.h"

/**
 * Constructor.
 */
PlatformTools::PlatformTools()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Destructor.
 */
PlatformTools::~PlatformTools()
{
  if (m_helpBrowser) {
    // Without close() the application will not quit when the main window is
    // closed but the help browser is still open.
    m_helpBrowser->close();
  }
}

/**
 * Get application settings.
 * @return settings instance.
 */
ISettings* PlatformTools::applicationSettings()
{
  return CorePlatformTools::applicationSettings();
}

/**
 * Get icon provider for tagged files.
 * @return icon provider.
 */
CoreTaggedFileIconProvider* PlatformTools::iconProvider()
{
  return GuiPlatformTools::iconProvider();
}

/**
 * Write text to clipboard.
 * @param text text to write
 * @return true if operation is supported.
 */
bool PlatformTools::writeToClipboard(const QString& text) const
{
  return GuiPlatformTools::writeToClipboard(text);
}

/**
 * Read text from clipboard.
 * @return text, null if operation not supported.
 */
QString PlatformTools::readFromClipboard() const
{
  return GuiPlatformTools::readFromClipboard();
}

/**
 * Create an audio player instance.
 * @param app application context
 * @param dbusEnabled true to enable MPRIS D-Bus interface
 * @return audio player, nullptr if not supported.
 */
QObject* PlatformTools::createAudioPlayer(Kid3Application* app,
                                   bool dbusEnabled) const
{
  return GuiPlatformTools::createAudioPlayer(app, dbusEnabled);
}

/**
 * Move file or directory to trash.
 *
 * @param path path to file or directory
 *
 * @return true if ok.
 */
bool PlatformTools::moveToTrash(const QString& path) const
{
  return CorePlatformTools::moveToTrash(path);
}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void PlatformTools::displayHelp(const QString& anchor)
{
  if (!m_helpBrowser) {
    const char* const kid3HandbookStr =
        QT_TRANSLATE_NOOP("@default", "Kid3 Handbook");
    QString caption(QCoreApplication::translate("@default", kid3HandbookStr));
    m_helpBrowser.reset(new BrowserDialog(nullptr, caption));
  }
  m_helpBrowser->goToAnchor(anchor);
  m_helpBrowser->setModal(!anchor.isEmpty());
  if (m_helpBrowser->isHidden()) {
    m_helpBrowser->show();
  }
}

/**
 * Get a themed icon by name.
 * @param name name of icon
 * @return icon.
 */
QIcon PlatformTools::iconFromTheme(const QString& name) const
{
  return QIcon::fromTheme(name,
      QIcon(QLatin1String(":/images/") + name + QLatin1String(".png")));
}

/**
 * Construct a name filter string suitable for file dialogs.
 * @param nameFilters list of description, filter pairs, e.g.
 * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
 * @return name filter string.
 */
QString PlatformTools::fileDialogNameFilter(
    const QList<QPair<QString, QString> >& nameFilters) const
{
  return CorePlatformTools::fileDialogNameFilter(nameFilters);
}

/**
 * Get file pattern part of m_nameFilter.
 * @param nameFilter name filter string
 * @return file patterns, e.g. "*.mp3".
 */
QString PlatformTools::getNameFilterPatterns(const QString& nameFilter) const
{
  return CorePlatformTools::getNameFilterPatterns(nameFilter);
}

/**
 * Display error dialog with item list.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 */
void PlatformTools::errorList(QWidget* parent, const QString& text,
    const QStringList& strlist, const QString& caption)
{
  MessageDialog::warningList(parent, caption, text, strlist, QMessageBox::Ok);
}

/**
 * Display warning dialog with yes, no, cancel buttons.
 * @param parent parent widget
 * @param text text
 * @param caption caption
 * @return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
 */
int PlatformTools::warningYesNoCancel(QWidget* parent, const QString& text,
    const QString& caption)
{
  return QMessageBox::warning(parent, caption, text,
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No,
                              QMessageBox::Cancel | QMessageBox::Escape);
}

/**
 * Display warning dialog with item list.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 * @return QMessageBox::Yes or QMessageBox::No.
 */
int PlatformTools::warningYesNoList(QWidget* parent, const QString& text,
    const QStringList& strlist, const QString& caption)
{
  return MessageDialog::warningList(parent, caption, text, strlist,
                                    QMessageBox::Yes | QMessageBox::No);
}

/**
 * Display dialog to select an existing file.
 * @param parent parent widget
 * @param caption caption
 * @param dir directory
 * @param filter filter
 * @param selectedFilter the selected filter is returned here
 * @return selected file, empty if canceled.
 */
QString PlatformTools::getOpenFileName(QWidget* parent, const QString& caption,
    const QString& dir, const QString& filter, QString* selectedFilter)
{
  return QFileDialog::getOpenFileName(
        parent, caption, dir, filter, selectedFilter,
        MainWindowConfig::instance().dontUseNativeDialogs()
        ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(nullptr));
}

/**
 * Display dialog to select existing files.
 * @param parent parent widget
 * @param caption caption
 * @param dir directory
 * @param filter filter
 * @param selectedFilter the selected filter is returned here
 * @return selected files, empty if canceled.
 */
QStringList PlatformTools::getOpenFileNames(QWidget* parent,
    const QString& caption, const QString& dir,
    const QString& filter, QString* selectedFilter)
{
  return QFileDialog::getOpenFileNames(
        parent, caption, dir, filter, selectedFilter,
        MainWindowConfig::instance().dontUseNativeDialogs()
        ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(nullptr));
}

/**
 * Display dialog to select a file to save.
 * @param parent parent widget
 * @param caption caption
 * @param dir directory
 * @param filter filter
 * @param selectedFilter the selected filter is returned here
 * @return selected file, empty if canceled.
 */
QString PlatformTools::getSaveFileName(QWidget* parent, const QString& caption,
    const QString& dir, const QString& filter, QString* selectedFilter)
{
  return QFileDialog::getSaveFileName(
        parent, caption, dir, filter, selectedFilter,
        MainWindowConfig::instance().dontUseNativeDialogs()
        ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(nullptr));
}

/**
 * Display dialog to select an existing directory.
 * @param parent parent widget
 * @param caption caption
 * @param startDir start directory
 * @return selected directory, empty if canceled.
 */
QString PlatformTools::getExistingDirectory(QWidget* parent,
    const QString& caption, const QString& startDir)
{
  return QFileDialog::getExistingDirectory(parent, caption, startDir,
      MainWindowConfig::instance().dontUseNativeDialogs()
      ? QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog
      : QFileDialog::ShowDirsOnly);
}

/**
 * Display warning dialog.
 * @param parent parent widget
 * @param text text
 * @param details detailed message
 * @param caption caption
 */
void PlatformTools::warningDialog(QWidget* parent,
    const QString& text, const QString& details, const QString& caption)
{
  MessageDialog dialog(parent);
  dialog.setWindowTitle(caption);
  dialog.setText(text);
  dialog.setInformativeText(details);
  dialog.setIcon(QMessageBox::Warning);
  dialog.exec();
}

/**
 * Display warning dialog with options to continue or cancel.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 * @return true if continue was selected.
 */
bool PlatformTools::warningContinueCancelList(QWidget* parent,
    const QString& text, const QStringList& strlist, const QString& caption)
{
  return MessageDialog::warningList(parent, caption, text, strlist) ==
      QMessageBox::Ok;
}
