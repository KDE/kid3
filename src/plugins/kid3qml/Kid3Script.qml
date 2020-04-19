/**
 * \file Kid3Script.qml
 * Base component for Kid3 user command scripts.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 27 Feb 2015
 *
 * Copyright (C) 2015-2017  Urs Fleisch
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

import QtQml 2.2 //@QtQuick2
//import QtQuick 2.2 //@QtQuick1
import Kid3 1.0

/**
 * @qmltype Kid3Script
 * @brief Base component for Kid3 user command scripts.
 *
 * This component can be used to write scripts which can be started as
 * user commands from Kid3 or started stand-alone using qml.
 * A minimal script could be:
 *
 * @code
 * import Kid3 1.0
 *
 * Kid3Script {
 *   onRun: {
 *     console.log("Hello world")
 *     Qt.quit()
 *   }
 * }
 * @endcode
 */
Timer {
  id: timer
  signal run

  /**
   * tagv1, tagv2, tagv2v1 can be used for the tag version with both
   * QtQuick 1 (Qt 4) and QtQuick 2 (Qt 5).
   * QtQuick 1 needs the enum types from C++, so a helper function is used.
   */
  //property variant tagv1: script.toTagVersion(Frame.TagV1) //@QtQuick1
  //property variant tagv2: script.toTagVersion(Frame.TagV2) //@QtQuick1
  //property variant tagv2v1: script.toTagVersion(Frame.TagV2V1) //@QtQuick1
  readonly property int tagv1: Frame.TagV1 //@QtQuick2
  readonly property int tagv2: Frame.TagV2 //@QtQuick2
  readonly property int tagv2v1: Frame.TagV2V1 //@QtQuick2

  // Container for sub module instances.
  property list<QtObject> _data: [
    ScriptUtils {
      id: _script
    },
    ConfigObjects {
      id: _configs
    }
  ]

  /** Access to ScriptUtils instance. */
  property alias script: _script
  /** Access to ConfigObjects instance. */
  property alias configs: _configs

  /** List of files to process with firstFile(), nextFile(). */
  property variant _paths: []
  /** Index of current file, -1 if _paths is not initialized yet. */
  property int _pathIndex: -1

  /**
   * Set list of files to process.
   * This function will be called by firstFile() if it has not been called
   * before. Calling it explicitly is only needed if the default file list
   * (from arguments if standalone, else from selected tagged files in file
   * list) is not appropriate.
   */
  function initFiles(paths) {
    _pathIndex = 0
    if (paths && paths.length > 0) {
      _paths = paths;
    } else if (isStandalone()) {
      _paths = getArguments();
    } else {
      _paths = app.getSelectedFilePaths();
    }
  }

  /**
   * To first file.
   * Will use the _paths list if available, else delegate to app.firstFile()
   * which will start iterating through all files.
   */
  function firstFile() {
    if (_pathIndex === -1) {
      initFiles()
    }
    if (_paths.length > 0) {
      _pathIndex = 0
      return nextFile()
    } else {
      return app.firstFile();
    }
  }

  /**
   * To next file.
   * firstFile() has to be called before. Returns false if at end of files.
   */
  function nextFile() {
    if (_paths.length > 0) {
      if (_pathIndex < _paths.length) {
        var path = _paths[_pathIndex++]
        return app.selectFile(path)
      } else {
        return false
      }
    } else  {
      return app.nextFile();
    }
  }

  /**
   * Get arguments after .qml script.
   * The arguments are passed to the script when the user command is
   * called from Kid3. In Qt 5, it is also possible to get arguments from
   * the command line when the script is invoked with qml or qmlscene.
   */
  function getArguments() {
    var params = []
    if (typeof args !== "undefined") {
      params = args.slice(0)
    } else if (Qt.application.arguments) {
      params = Qt.application.arguments.slice(0)
    }
    while (params.length > 0) {
      var p = params.shift()
      if (p.substr(-4) === ".qml") {
        if (params.length > 0 && params[0] === "--") {
          params.shift()
        }
        break
      }
    }
    return params
  }

  /**
   * Signal run() after the directory @a paths has been opened.
   * This function is used when the script is called stand-alone and
   * the directory is passed as a command line parameter.
   */
  function openDirectory(paths) {
    function onDirectoryOpened() {
      app.directoryOpened.disconnect(onDirectoryOpened)
      run()
    }
    app.directoryOpened.connect(onDirectoryOpened)
    app.openDirectory(paths)
  }

  /**
   * Check if script is running stand-alone, i.e. not in the Kid3 application.
   */
  function isStandalone() {
    return typeof args === "undefined"
  }

  /**
   * Main function.
   * When the script is run as a user command from Kid3, simply run() is
   * signalled. When the script is called from the command line, run() is
   * signalled after opening the directory.
   */
  function main() {
    if (!isStandalone()) {
      // Started as a user action from Kid3.
      run()
    } else {
      // Started as a QML script outside of Kid3, start in current directory.
      app.selectedFilesUpdated.connect(app.tagsToFrameModels)
      app.selectedFilesChanged.connect(app.tagsToFrameModels)
      app.readConfig()
      openDirectory(".")
    }
  }

  /**
   * Invoke a callback with delay in ms and optional arguments.
   * This can be used to keep the GUI responsive by splitting the code
   * into asynchronous functions and call them after a small delay.
   */
  function setTimeout(callback, delay) {
    var argv = Array.prototype.slice.call(arguments, 2)

    function timeoutHandler() {
      triggered.disconnect(timeoutHandler)
      callback.apply(null, argv)
    }

    triggered.connect(timeoutHandler)
    interval = delay
    if (!running) {
      start()
    } else {
      // QtQuick 1 cannot restart a timer from a handler, see QTBUG-22004,
      // https://bugreports.qt.io/browse/QTBUG-22004
      // This workaround starts it when it is possible again.
      function restartHandler() {
        runningChanged.disconnect(restartHandler)
        if (!running) {
          start()
        }
      }

      runningChanged.connect(restartHandler)
    }
  }

  Component.onCompleted: setTimeout(main, 1)
}
