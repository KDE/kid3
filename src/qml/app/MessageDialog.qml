/**
 * \file MessageDialog.qml
 * Message dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import "../componentsqtquick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu

Dialog {
  id: page

  signal yes
  signal no
  signal rejected

  Row {
    spacing: constants.spacing
    Button {
      width: (parent.width - 2 * parent.spacing) / 3
      id: yesButton
      text: qsTr("Yes")
      onClicked: {
        page.hide()
        page.yes()
      }
    }
    Button {
      width: (parent.width - 2 * parent.spacing) / 3
      text: qsTr("No")
      onClicked: {
        page.hide()
        page.no()
      }
    }
    Button {
      width: (parent.width - 2 * parent.spacing) / 3
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
        page.rejected()
      }
    }
  }
}
