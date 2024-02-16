/**
 * \file kdeplatformtools.cpp
 * KDE platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Mar 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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

#include "kdeplatformtools.h"
#include <QCoreApplication>
#include <QtConfig>
#include <kconfig_version.h>
#include <kwidgetsaddons_version.h>
#include <KMessageBox>
#include <KSharedConfig>
#include <KIO/CopyJob>
#include <QUrl>
#include <QFileDialog>
#include <QDesktopServices>
#include "mainwindowconfig.h"
#include "kdesettings.h"

/**
 * Constructor.
 */
KdePlatformTools::KdePlatformTools()
{}

/**
 * Destructor.
 */
KdePlatformTools::~KdePlatformTools()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Get application settings.
 * @return settings instance.
 */
ISettings* KdePlatformTools::applicationSettings()
{
  if (!m_config) {
    auto cfg = KSharedConfig::openConfig();
#if KCONFIG_VERSION >= 0x054300
    auto stateCfg = KSharedConfig::openStateConfig();
#else
    auto stateCfg = cfg;
#endif
    m_config.reset(new KdeSettings(cfg, stateCfg));
  }
  return m_config.data();
}

/**
 * Get icon provider for tagged files.
 * @return icon provider.
 */
CoreTaggedFileIconProvider* KdePlatformTools::iconProvider()
{
  return GuiPlatformTools::iconProvider();
}

/**
 * Write text to clipboard.
 * @param text text to write
 * @return true if operation is supported.
 */
bool KdePlatformTools::writeToClipboard(const QString& text) const
{
  return GuiPlatformTools::writeToClipboard(text);
}

/**
 * Read text from clipboard.
 * @return text, null if operation not supported.
 */
QString KdePlatformTools::readFromClipboard() const
{
  return GuiPlatformTools::readFromClipboard();
}

/**
 * Create an audio player instance.
 * @param app application context
 * @param dbusEnabled true to enable MPRIS D-Bus interface
 * @return audio player, nullptr if not supported.
 */
QObject* KdePlatformTools::createAudioPlayer(Kid3Application* app,
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
bool KdePlatformTools::moveToTrash(const QString& path) const
{
  KIO::Job* job = KIO::trash(QUrl::fromLocalFile(path));
  return job->exec();
}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void KdePlatformTools::displayHelp(const QString& anchor)
{
  QUrl url(QLatin1String("help:/kid3/index.html"));
  if (!anchor.isEmpty()) {
    url.setFragment(anchor);
  }
  QDesktopServices::openUrl(url);
}

/**
 * Get a themed icon by name.
 * @param name name of icon
 * @return icon.
 */
QIcon KdePlatformTools::iconFromTheme(const QString& name) const
{
  return QIcon::fromTheme(name);
}

/**
 * Construct a name filter string suitable for file dialogs.
 * @param nameFilters list of description, filter pairs, e.g.
 * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
 * @return name filter string.
 */
QString KdePlatformTools::fileDialogNameFilter(
    const QList<QPair<QString, QString> >& nameFilters) const
{
  return ICorePlatformTools::qtFileDialogNameFilter(nameFilters);
}

/**
 * Get file pattern part of m_nameFilter.
 * @param nameFilter name filter string
 * @return file patterns, e.g. "*.mp3".
 */
QString KdePlatformTools::getNameFilterPatterns(const QString& nameFilter) const
{
  return ICorePlatformTools::qtNameFilterPatterns(nameFilter);
}

/**
 * Display error dialog with item list.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 */
void KdePlatformTools::errorList(QWidget* parent, const QString& text,
    const QStringList& strlist, const QString& caption)
{
  KMessageBox::errorList(parent, text, strlist, caption);
}

/**
 * Display warning dialog with yes, no, cancel buttons.
 * @param parent parent widget
 * @param text text
 * @param caption caption
 * @return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
 */
int KdePlatformTools::warningYesNoCancel(QWidget* parent, const QString& text,
                                         const QString& caption)
{
#if KWIDGETSADDONS_VERSION >= 0x05f000
switch (KMessageBox::warningTwoActionsCancel(parent, text, caption,
          KGuiItem(QCoreApplication::translate("@default", "&Yes")),
          KGuiItem(QCoreApplication::translate("@default", "&No")))) {
  case KMessageBox::Ok:
    return QMessageBox::Ok;
  case KMessageBox::Cancel:
    return QMessageBox::Cancel;
  case KMessageBox::PrimaryAction:
    return QMessageBox::Yes;
  case KMessageBox::SecondaryAction:
    return QMessageBox::No;
  case KMessageBox::Continue:
  default:
    return QMessageBox::Ignore;
  }
#else
  switch (KMessageBox::warningYesNoCancel(parent, text, caption)) {
  case KMessageBox::Ok:
    return QMessageBox::Ok;
  case KMessageBox::Cancel:
    return QMessageBox::Cancel;
  case KMessageBox::Yes:
    return QMessageBox::Yes;
  case KMessageBox::No:
    return QMessageBox::No;
  case KMessageBox::Continue:
  default:
    return QMessageBox::Ignore;
  }
#endif
}

/**
 * Display warning dialog with item list.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 * @return QMessageBox::Yes or QMessageBox::No.
 */
int KdePlatformTools::warningYesNoList(QWidget* parent, const QString& text,
    const QStringList& strlist, const QString& caption)
{
#if KWIDGETSADDONS_VERSION >= 0x05f000
  switch (KMessageBox::warningTwoActionsList(parent, text, strlist, caption,
            KGuiItem(QCoreApplication::translate("@default", "&Yes")),
            KGuiItem(QCoreApplication::translate("@default", "&No")))) {
  case KMessageBox::PrimaryAction:
    return QMessageBox::Yes;
  case KMessageBox::SecondaryAction:
  default:
    return QMessageBox::No;
  }
#else
  switch (KMessageBox::warningYesNoList(parent, text, strlist, caption)) {
  case KMessageBox::Yes:
    return QMessageBox::Yes;
  case KMessageBox::No:
  default:
    return QMessageBox::No;
  }
#endif
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
QString KdePlatformTools::getOpenFileName(QWidget* parent,
    const QString& caption, const QString& dir, const QString& filter,
    QString* selectedFilter)
{
  return QFileDialog::getOpenFileName(
        parent, caption, dir, filter, selectedFilter,
        MainWindowConfig::instance().dontUseNativeDialogs()
        ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
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
QStringList KdePlatformTools::getOpenFileNames(QWidget* parent,
    const QString& caption, const QString& dir,
    const QString& filter, QString* selectedFilter)
{
  return QFileDialog::getOpenFileNames(
        parent, caption, dir, filter, selectedFilter,
        MainWindowConfig::instance().dontUseNativeDialogs()
        ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
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
QString KdePlatformTools::getSaveFileName(QWidget* parent,
    const QString& caption, const QString& dir, const QString& filter,
    QString* selectedFilter)
{
  return QFileDialog::getSaveFileName(
        parent, caption, dir, filter, selectedFilter,
        MainWindowConfig::instance().dontUseNativeDialogs()
        ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
}

/**
 * Display dialog to select an existing directory.
 * @param parent parent widget
 * @param caption caption
 * @param startDir start directory
 * @return selected directory, empty if canceled.
 */
QString KdePlatformTools::getExistingDirectory(QWidget* parent,
    const QString& caption, const QString& startDir)
{
  return QFileDialog::getExistingDirectory(parent, caption, startDir,
      MainWindowConfig::instance().dontUseNativeDialogs()
      ? QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog
      : QFileDialog::ShowDirsOnly);
}

/**
 * Check if platform has a graphical user interface.
 * @return true if platform has GUI.
 */
bool KdePlatformTools::hasGui() const
{
  return true;
}

/**
 * Display warning dialog.
 * @param parent parent widget
 * @param text text
 * @param details detailed message
 * @param caption caption
 */
void KdePlatformTools::warningDialog(QWidget* parent,
    const QString& text, const QString& details, const QString& caption)
{
  KMessageBox::error(parent, text + details, caption);
}

/**
 * Display warning dialog with options to continue or cancel.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 * @return true if continue was selected.
 */
bool KdePlatformTools::warningContinueCancelList(QWidget* parent,
    const QString& text, const QStringList& strlist, const QString& caption)
{
  return KMessageBox::warningContinueCancelList(parent, text, strlist, caption,
      KStandardGuiItem::ok(), KStandardGuiItem::cancel(), QString(),
      KMessageBox::Dangerous) == KMessageBox::Continue;
}
