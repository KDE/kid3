/**
 * \file AboutDialog.qml
 * About dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 7 Nov 2015
 *
 * Copyright (C) 2015-2018  Urs Fleisch
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

import QtQuick 2.9
import QtQuick.Controls 2.2

Dialog {
  id: page
  title: qsTr("About Kid3")
  modal: true
  x: (root.width - width) / 2
  y: root.height / 6
  width: Math.min(root.width, constants.gu(40))

  Label {
    id: textLabel
    wrapMode: Text.WordWrap
    text: "<big><b>Kid3 " + script.getKid3Version() +
          "</b></big><br/><br/>" + qsTr("Audio Tag Editor") +
          "<br/><br/>(c) 2003-" + script.getKid3ReleaseYear() +
          " <a href=\"mailto:ufleisch@users.sourceforge.net\">Urs Fleisch</a>" +
          "<br/><br/>" +
          "<a href=\"http://kid3.sourceforge.net/\">" +
          "http://kid3.sourceforge.net</a><br/>" + qsTr("License") +
          ": <a href=\"http://www.gnu.org/licenses/licenses.html#GPL\">" +
          "GNU General Public License</a><br/><br/>" + qsTr(
            "This program uses <a href=\"http://www.qt.io/\">Qt</a> version %1."
            ).arg(script.getQtVersion())
    onLinkActivated: Qt.openUrlExternally(link)
  }
}
