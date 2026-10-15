/**
 * \file accessibilityhelper.h
 * Workarounds to improve screen reader support of item views.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2026
 *
 * Copyright (C) 2026  Urs Fleisch
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

#include <QModelIndex>

#if defined Q_OS_MACOS && QT_VERSION >= 0x060800 && !defined QT_NO_ACCESSIBILITY

#include "kid3api.h"

class QAbstractItemView;
class QComboBox;
class QMenu;
class QString;
class QWidget;

namespace AccessibilityHelper {

/**
 * Improve the support of VoiceOver for an item view on macOS.
 *
 * - Editors opened inside the view are made reachable by VoiceOver.
 * - The current item is announced when it is changed by the user.
 *
 * The selection model of the view has to be set before calling this function.
 *
 * @param view item view
 */
KID3_GUI_EXPORT void setupItemView(QAbstractItemView* view);

/**
 * Make VoiceOver speak a message on macOS.
 * @param widget widget which changed
 * @param message text to be spoken
 */
KID3_GUI_EXPORT void announce(QWidget* widget, const QString& message);

/**
 * Make VoiceOver speak the description of an item of a view on macOS.
 * @param view item view
 * @param index index of item
 */
KID3_GUI_EXPORT void announceItem(QAbstractItemView* view,
                                  const QModelIndex& index);

/**
 * Improve the support of VoiceOver for a combo box on macOS.
 *
 * - The items of the popup are announced.
 * - The current item is announced when it is changed with the cursor keys
 *   while the popup is closed.
 *
 * May be called multiple times for the same combo box.
 *
 * @param comboBox combo box
 */
KID3_GUI_EXPORT void setupComboBox(QComboBox* comboBox);

/**
 * Improve the support of VoiceOver for a popup menu on macOS.
 *
 * The popup menus of Qt are separate windows which are not followed by
 * VoiceOver, so the menu is announced when it is opened and its items
 * are announced when they become active. This includes the submenus,
 * so it has to be called only for the top level menu.
 *
 * May be called multiple times for the same menu.
 *
 * @param menu menu
 */
KID3_GUI_EXPORT void setupMenu(QMenu* menu);

}

#endif
