/**
 * \file kid3mainwindow.h
 * Kid3 main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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

#pragma once

#include <QMainWindow>
#include "basemainwindow.h"

class QAction;
class QSessionManager;
class RecentFilesMenu;
class ShortcutsModel;
class Kid3ApplicationTagContext;

/**
 * Kid3 main window.
 */
class Kid3MainWindow : public QMainWindow, public BaseMainWindow {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform specific tools
   * @param app application context
   * @param parent parent widget
   */
  explicit Kid3MainWindow(IPlatformTools* platformTools, Kid3Application* app,
                          QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~Kid3MainWindow() override = default;

  /**
   * Init menu and toolbar actions.
   */
  virtual void initActions() override;

  /**
   * Get keyboard shortcuts.
   * @return mapping of action names to key sequences.
   */
  virtual QMap<QString, QKeySequence> shortcutsMap() const override;

  /**
   * Add directory to recent files list.
   *
   * @param dirName path to directory
   */
  virtual void addDirectoryToRecentFiles(const QString& dirName) override;

  /**
   * Read settings from the configuration.
   */
  virtual void readConfig() override;

  /**
   * Store geometry and recent files in settings.
   */
  virtual void saveConfig() override;

  /**
   * Get action for Settings/Auto Hide Tags.
   * @return action.
   */
  virtual QAction* autoHideTagsAction() override;

  /**
   * Get action for Settings/Hide Picture.
   * @return action.
   */
  virtual QAction* showHidePictureAction() override;

  /**
   * Set main window caption.
   *
   * @param caption caption without application name
   * @param modified true if any file is modified
   */
  virtual void setWindowCaption(const QString& caption, bool modified) override;

protected:
  /**
   * Window is closed.
   *
   * @param ce close event
   */
  virtual void closeEvent(QCloseEvent* ce) override;

private slots:
  /**
   * Open recent directory.
   *
   * @param dir directory to open
   */
  void slotFileOpenRecentDirectory(const QString& dir);

  /**
   * Turn status bar on or off.
   */
  void slotViewStatusBar();

  /**
   * Preferences.
   */
  void slotSettingsConfigure();

  /**
   * Display handbook.
   */
  void slotHelpHandbook();

  /**
   * Display "About" dialog.
   */
  void slotHelpAbout();

  /**
   * Display "About Qt" dialog.
   */
  void slotHelpAboutQt();

  /**
   * Called when session manager wants application to commit its data.
   * @param manager session manager
   */
  void onCommitDataRequest(QSessionManager& manager);

  /**
   * Add user action to shortcuts.
   * @param name name of action
   * @param action action to add
   */
  void onUserActionAdded(const QString& name, QAction* action);

  /**
   * Remove user action from shortcuts.
   * @param name name of action
   * @param action action to remove
   */
  void onUserActionRemoved(const QString& name, QAction* action);

private:
  /**
   * Read font and style options.
   */
  void readFontAndStyleOptions();

  /**
   * Init actions of form.
   */
  void initFormActions();

  /**
   * Add action of form.
   */
  QAction* addFormAction(const QString& text, const QString& name,
                         const QString& context);

  IPlatformTools* m_platformTools;
  RecentFilesMenu* m_fileOpenRecent;
  ShortcutsModel* m_shortcutsModel;
  QAction* m_viewToolBar;
  QAction* m_viewStatusBar;
  QAction* m_settingsAutoHideTags;
  QAction* m_settingsShowHidePicture;
};
