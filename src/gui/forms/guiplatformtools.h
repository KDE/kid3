/**
 * \file guiplatformtools.h
 * Platform specific tools for QtGui (without QtWidget).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Jul 2019
 *
 * Copyright (C) 2019-2024  Urs Fleisch
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

#include "coreplatformtools.h"

/**
 * Platform specific tools for QtGui (without QtWidget).
 */
class KID3_GUI_EXPORT GuiPlatformTools : public CorePlatformTools {
public:
  /**
   * Destructor.
   */
  ~GuiPlatformTools() override;

  /**
   * Get icon provider for tagged files.
   * @return icon provider.
   */
  CoreTaggedFileIconProvider* iconProvider() override;

  /**
   * Write text to clipboard.
   * @param text text to write
   * @return true if operation is supported.
   */
  bool writeToClipboard(const QString& text) const override;

  /**
   * Read text from clipboard.
   * @return text, null if operation not supported.
   */
  QString readFromClipboard() const override;

  /**
   * Create an audio player instance.
   * @param app application context
   * @param dbusEnabled true to enable MPRIS D-Bus interface
   * @return audio player, nullptr if not supported.
   */
  QObject* createAudioPlayer(Kid3Application* app,
                             bool dbusEnabled) const override;

private:
  QScopedPointer<CoreTaggedFileIconProvider> m_iconProvider;
};
