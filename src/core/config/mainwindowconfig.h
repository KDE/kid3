/**
 * \file mainwindowconfig.h
 * Main window configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 08 Apr 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "generalconfig.h"
#include "kid3api.h"

/**
 * Main window configuration.
 */
class KID3_CORE_EXPORT MainWindowConfig : public StoredConfig<MainWindowConfig> {
  Q_OBJECT
  /** mainwindow geometry */
  Q_PROPERTY(QByteArray geometry READ geometry WRITE setGeometry NOTIFY geometryChanged)
  /** mainwindow state */
  Q_PROPERTY(QByteArray windowState READ windowState WRITE setWindowState NOTIFY windowStateChanged)
  /** custom application font family */
  Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
  /** custom application font size */
  Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
  /** custom application style, empty if not used */
  Q_PROPERTY(QString style READ style WRITE setStyle NOTIFY styleChanged)
  /** true if custom application font is used */
  Q_PROPERTY(bool useFont READ useFont WRITE setUseFont NOTIFY useFontChanged)
  /** true to hide toolbar */
  Q_PROPERTY(bool hideToolBar READ hideToolBar WRITE setHideToolBar NOTIFY hideToolBarChanged)
  /** true to hide statusbar */
  Q_PROPERTY(bool hideStatusBar READ hideStatusBar WRITE setHideStatusBar NOTIFY hideStatusBarChanged)
  /** Don't use the native file dialog if true */
  Q_PROPERTY(bool dontUseNativeDialogs READ dontUseNativeDialogs WRITE setDontUseNativeDialogs NOTIFY dontUseNativeDialogsChanged)

public:
  /**
   * Constructor.
   */
  MainWindowConfig();

  /**
   * Destructor.
   */
  virtual ~MainWindowConfig() override = default;

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  virtual void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  virtual void readFromConfig(ISettings* config) override;

  /** Get mainwindow geometry. */
  QByteArray geometry() const { return m_geometry; }

  /** Set mainwindow geometry. */
  void setGeometry(const QByteArray& geometry);

  /** Get mainwindow state. */
  QByteArray windowState() const { return m_windowState; }

  /** Set mainwindow state. */
  void setWindowState(const QByteArray& windowState);

  /** Get custom application font family. */
  QString fontFamily() const { return m_fontFamily; }

  /** Set custom application font family. */
  void setFontFamily(const QString& fontFamily);

  /** Get custom application font size. */
  int fontSize() const { return m_fontSize; }

  /** Set custom application font size. */
  void setFontSize(int fontSize);

  /** Get custom application style, empty if not used. */
  QString style() const { return m_style; }

  /** Set custom application style. */
  void setStyle(const QString& style);

  /** Check if custom application font is used. */
  bool useFont() const { return m_useFont; }

  /** Set if custom application font is used. */
  void setUseFont(bool useFont);

  /** Check if toolbar is hidden. */
  bool hideToolBar() const { return m_hideToolBar; }

  /** Set if hide toolbar is hidden. */
  void setHideToolBar(bool hideToolBar);

  /** Check if statusbar is hidden. */
  bool hideStatusBar() const { return m_hideStatusBar; }

  /** Set if statusbar is hidden. */
  void setHideStatusBar(bool hideStatusBar);

  /** Check if native file dialogs are used. */
  bool dontUseNativeDialogs() const { return m_dontUseNativeDialogs; }

  /** Set if native file dialogs are used. */
  void setDontUseNativeDialogs(bool dontUseNativeDialogs);

signals:
  /** Emitted when @a geometry changed. */
  void geometryChanged(const QByteArray& geometry);

  /** Emitted when @a windowState changed. */
  void windowStateChanged(const QByteArray& windowState);

  /** Emitted when @a fontFamily changed. */
  void fontFamilyChanged(const QString& fontFamily);

  /** Emitted when @a fontSize changed. */
  void fontSizeChanged(int fontSize);

  /** Emitted when @a style changed. */
  void styleChanged(const QString& style);

  /** Emitted when @a useFont changed. */
  void useFontChanged(bool useFont);

  /** Emitted when @a hideToolBar changed. */
  void hideToolBarChanged(bool hideToolBar);

  /** Emitted when @a hideStatusBar changed. */
  void hideStatusBarChanged(bool hideStatusBar);

  /** Emitted when @a dontUseNativeDialogs changed. */
  void dontUseNativeDialogsChanged(bool dontUseNativeDialogs);

private:
  friend MainWindowConfig& StoredConfig<MainWindowConfig>::instance();

  QByteArray m_geometry;
  QByteArray m_windowState;
  QString m_fontFamily;
  int m_fontSize;
  QString m_style;
  bool m_useFont;
  bool m_hideToolBar;
  bool m_hideStatusBar;
  bool m_dontUseNativeDialogs;

  /** Index in configuration storage */
  static int s_index;
};
