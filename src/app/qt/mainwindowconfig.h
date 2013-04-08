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

#ifndef MAINWINDOWCONFIG_H
#define MAINWINDOWCONFIG_H

#include "generalconfig.h"

/**
 * Main window configuration.
 */
class MainWindowConfig : public GeneralConfig
{
public:
  /**
   * Constructor.
   */
  MainWindowConfig(const QString& group);

  /**
   * Destructor.
   */
  virtual ~MainWindowConfig();

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  void writeToConfig(ISettings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  void readFromConfig(ISettings* config);


  /** true to hide toolbar */
  bool m_hideToolBar;
  /** true to hide statusbar */
  bool m_hideStatusBar;
  /** mainwindow geometry */
  QByteArray m_geometry;
  /** mainwindow state */
  QByteArray m_windowState;
  /** true if custom application font is used */
  bool m_useFont;
  /** custom application font family */
  QString m_fontFamily;
  /** custom application font size */
  int m_fontSize;
  /** custom application style, empty if not used */
  QString m_style;
  /** Don't use the native file dialog if true */
  bool m_dontUseNativeDialogs;
};

#endif
