/**
 * \file configdialog.h
 * Configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2012  Urs Fleisch
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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

class QTreeView;
class QLabel;
class QCheckBox;
class QString;
class QWidget;
class QComboBox;
class ShortcutsModel;
class MainWindowConfig;
class ConfigDialogPages;
class IPlatformTools;

/**
 * Configuration dialog.
 */
class ConfigDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform specific tools
   * @param parent  parent widget
   * @param caption dialog title
   * @param shortcutsModel shortcuts model
   */
  ConfigDialog(IPlatformTools* platformTools, QWidget* parent,
               QString& caption, ShortcutsModel* shortcutsModel);

  /**
   * Destructor.
   */
  virtual ~ConfigDialog() override = default;

  /**
   * Set values in dialog from current configuration.
   */
  void setConfig();

  /**
   * Get values from dialog and store them in the current configuration.
   */
  void getConfig() const;

protected slots:
  /**
   * Show help.
   */
  void slotHelp();

  /**
   * Display warning that keyboard shortcut is already used.
   *
   * @param key string representation of key sequence
   * @param context context of action
   * @param action action using @a key
   */
  void warnAboutAlreadyUsedShortcut(const QString& key, const QString& context,
                                    const QAction* action);

  /**
   * Clear warning about already used keyboard shortcut.
   */
  void clearAlreadyUsedShortcutWarning();

  /**
   * Set additional configurations to their defaults.
   */
  void setDefaultConfig();

  /**
   * Select custom application font.
   */
  void slotSelectFont();

  /**
   * Select custom application style.
   *
   * @param key style key
   */
  void slotSelectStyle(const QString& key);

  /**
   * Revert the font and style to the values in the settings.
   */
  void slotRevertFontAndStyle();

private:
  void setConfigs(const MainWindowConfig& mainWindowConfig);

  ConfigDialogPages* m_pages;
  ShortcutsModel* m_shortcutsModel;
  QTreeView* m_shortcutsTreeView;
  QLabel* m_shortcutAlreadyUsedLabel;
  QCheckBox* m_useApplicationFontCheckBox;
  QPushButton* m_applicationFontButton;
  QCheckBox* m_useApplicationStyleCheckBox;
  QComboBox* m_applicationStyleComboBox;
  QCheckBox* m_useNativeDialogsCheckBox;
  QFont m_font;
  QString m_style;
  bool m_fontChanged;
  bool m_styleChanged;
};

#endif
