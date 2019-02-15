/**
 * \file mainwindowconfig.cpp
 * Main window configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 08 Apr 2013
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

#include "mainwindowconfig.h"
#include "isettings.h"

int MainWindowConfig::s_index = -1;

/**
 * Constructor.
 */
MainWindowConfig::MainWindowConfig()
  : StoredConfig<MainWindowConfig>(QLatin1String("MainWindow")),
    m_fontSize(-1),
    m_useFont(false),
    m_hideToolBar(false),
    m_hideStatusBar(false),
    m_dontUseNativeDialogs(
#if defined Q_OS_WIN32 || defined Q_OS_MAC
      false
#else
      true
#endif
    )
{
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void MainWindowConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("HideToolBar"), QVariant(m_hideToolBar));
  config->setValue(QLatin1String("HideStatusBar"), QVariant(m_hideStatusBar));
  config->setValue(QLatin1String("Geometry"), m_geometry);
  config->setValue(QLatin1String("WindowState"), m_windowState);
  config->setValue(QLatin1String("UseFont"), QVariant(m_useFont));
  config->setValue(QLatin1String("FontFamily"), QVariant(m_fontFamily));
  config->setValue(QLatin1String("FontSize"), QVariant(m_fontSize));
  config->setValue(QLatin1String("Style"), QVariant(m_style));
  config->setValue(QLatin1String("DontUseNativeDialogs"), QVariant(m_dontUseNativeDialogs));
  config->setValue(QLatin1String("QtQuickStyle"), QVariant(m_qtQuickStyle));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void MainWindowConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_hideToolBar = config->value(QLatin1String("HideToolBar"), m_hideToolBar).toBool();
  m_hideStatusBar = config->value(QLatin1String("HideStatusBar"), m_hideStatusBar).toBool();
  m_geometry = config->value(QLatin1String("Geometry"), m_geometry).toByteArray();
  m_windowState = config->value(QLatin1String("WindowState"), m_windowState).toByteArray();
  m_useFont = config->value(QLatin1String("UseFont"), m_useFont).toBool();
  m_fontFamily = config->value(QLatin1String("FontFamily"), m_fontFamily).toString();
  m_fontSize = config->value(QLatin1String("FontSize"), -1).toInt();
  m_style = config->value(QLatin1String("Style"), m_style).toString();
  m_dontUseNativeDialogs = config->value(QLatin1String("DontUseNativeDialogs"),
                                         m_dontUseNativeDialogs).toBool();
  m_qtQuickStyle = config->value(QLatin1String("QtQuickStyle"), m_qtQuickStyle).toString();
  config->endGroup();
}

void MainWindowConfig::setGeometry(const QByteArray& geometry)
{
  if (m_geometry != geometry) {
    m_geometry = geometry;
    emit geometryChanged(m_geometry);
  }
}

void MainWindowConfig::setWindowState(const QByteArray& windowState)
{
  if (m_windowState != windowState) {
    m_windowState = windowState;
    emit windowStateChanged(m_windowState);
  }
}

void MainWindowConfig::setFontFamily(const QString& fontFamily)
{
  if (m_fontFamily != fontFamily) {
    m_fontFamily = fontFamily;
    emit fontFamilyChanged(m_fontFamily);
  }
}

void MainWindowConfig::setFontSize(int fontSize)
{
  if (m_fontSize != fontSize) {
    m_fontSize = fontSize;
    emit fontSizeChanged(m_fontSize);
  }
}

void MainWindowConfig::setStyle(const QString& style)
{
  if (m_style != style) {
    m_style = style;
    emit styleChanged(m_style);
  }
}

void MainWindowConfig::setUseFont(bool useFont)
{
  if (m_useFont != useFont) {
    m_useFont = useFont;
    emit useFontChanged(m_useFont);
  }
}

void MainWindowConfig::setHideToolBar(bool hideToolBar)
{
  if (m_hideToolBar != hideToolBar) {
    m_hideToolBar = hideToolBar;
    emit hideToolBarChanged(m_hideToolBar);
  }
}

void MainWindowConfig::setHideStatusBar(bool hideStatusBar)
{
  if (m_hideStatusBar != hideStatusBar) {
    m_hideStatusBar = hideStatusBar;
    emit hideStatusBarChanged(m_hideStatusBar);
  }
}

void MainWindowConfig::setDontUseNativeDialogs(bool dontUseNativeDialogs)
{
  if (m_dontUseNativeDialogs != dontUseNativeDialogs) {
    m_dontUseNativeDialogs = dontUseNativeDialogs;
    emit dontUseNativeDialogsChanged(m_dontUseNativeDialogs);
  }
}

void MainWindowConfig::setQtQuickStyle(const QString& qtQuickStyle)
{
  if (m_qtQuickStyle != qtQuickStyle) {
    m_qtQuickStyle = qtQuickStyle;
    emit styleChanged(m_qtQuickStyle);
  }
}

QStringList MainWindowConfig::getQtQuickStyleNames()
{
  return {
    QLatin1String("Material/Light"),
    QLatin1String("Material/Dark"),
    QLatin1String("Material/System")
  };
}
