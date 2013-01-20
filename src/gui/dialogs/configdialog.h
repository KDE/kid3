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

#include "config.h"

#include <QStringList>
#ifdef CONFIG_USE_KDE
#include <kconfigdialog.h>
#else
#include <QDialog>
#endif
#include "formatconfig.h"

class FormatBox;
class QCheckBox;
class FormatConfig;
class MiscConfig;
class ConfigTable;
class CommandsTableModel;
class QString;
class QWidget;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QStringListModel;
class CheckableStringListModel;
class ConfigStore;

/** Base class for configuration dialog. */
#ifdef CONFIG_USE_KDE
typedef KConfigDialog ConfigDialogBaseClass;
#else
typedef QDialog ConfigDialogBaseClass;

class QTreeView;
class QLabel;
class ShortcutsModel;
#endif

/**
 * Configuration dialog.
 */
class ConfigDialog : public ConfigDialogBaseClass
{
Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent  parent widget
   * @param caption dialog title
   */
#ifdef CONFIG_USE_KDE
  ConfigDialog(QWidget* parent, QString& caption,
               KConfigSkeleton* configSkeleton);
#else
  ConfigDialog(QWidget* parent, QString& caption);
#endif

  /**
   * Destructor.
   */
  ~ConfigDialog();

  /**
   * Set values in dialog from current configuration.
   *
   * @param cfg configuration
   */
  void setConfig(const ConfigStore* cfg);

  /**
   * Get values from dialog and store them in the current configuration.
   *
   * @param cfg configuration
   */
  void getConfig(ConfigStore* cfg) const;

protected slots:
  /**
   * Show help.
   */
  virtual void slotHelp();

#ifndef CONFIG_USE_KDE
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
#endif

private:
#ifndef CONFIG_USE_KDE
  void setShortcutsModel(ShortcutsModel* model);
#endif
  /** Load last-opened files checkbox */
  QCheckBox* m_loadLastOpenedFileCheckBox;
  /** Preserve timestamp checkbox */
  QCheckBox* m_preserveTimeCheckBox;
  /** Mark changes checkbox */
  QCheckBox* m_markChangesCheckBox;
  /** Filename for cover lineedit */
  QLineEdit* m_coverFileNameLineEdit;
  /** Mark truncated fields checkbox */
  QCheckBox* m_markTruncationsCheckBox;
  /** ID3v1 text encodings */
  QStringList m_textEncodingV1List;
  /** ID3v1 text encoding combo box */
  QComboBox* m_textEncodingV1ComboBox;
  /** Use track/total number of tracks format checkbox */
  QCheckBox* m_totalNumTracksCheckBox;
  /** Comment field name combo box */
  QComboBox* m_commentNameComboBox;
  /** Picture field name combo box */
  QComboBox* m_pictureNameComboBox;
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
  /** Genre as text instead of numeric string checkbox */
  QCheckBox* m_genreNotNumericCheckBox;
  /** ID3v2 text encoding combo box */
  QComboBox* m_textEncodingComboBox;
#endif
#if defined HAVE_TAGLIB && (defined HAVE_ID3LIB || defined HAVE_TAGLIB_ID3V23_SUPPORT)
  /** ID3v2 version combo box */
  QComboBox* m_id3v2VersionComboBox;
#endif
  /** Number of digits in track number spin box */
  QSpinBox* m_trackNumberDigitsSpinBox;
  /** Filename Format box */
  FormatBox* m_fnFormatBox;
  /** ID3 Format box */
  FormatBox* m_id3FormatBox;
  /** Only custom genres checkbox */
  QCheckBox* m_onlyCustomGenresCheckBox;
  /** Model with list of custom genres */
  QStringListModel* m_genresEditModel;
  /** Model with standard tags selection */
  CheckableStringListModel* m_quickAccessTagsModel;
  /** Play on double click checkbox */
  QCheckBox* m_playOnDoubleClickCheckBox;
  /** Commands table */
  ConfigTable* m_commandsTable;
  /** Commands table model */
  CommandsTableModel* m_commandsTableModel;
  /** Browser line edit */
  QLineEdit* m_browserLineEdit;
  /** Use proxy check box */
  QCheckBox* m_proxyCheckBox;
  /** Proxy line edit */
  QLineEdit* m_proxyLineEdit;
  /** Use proxy authentication check box */
  QCheckBox* m_proxyAuthenticationCheckBox;
  /** Proxy user name line edit */
  QLineEdit* m_proxyUserNameLineEdit;
  /** Proxy password line edit */
  QLineEdit* m_proxyPasswordLineEdit;
#ifndef CONFIG_USE_KDE
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
#endif
};

#endif
