/**
 * \file qmlcommandplugin.cpp
 * Starter for QML scripts.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Feb 2015
 *
 * Copyright (C) 2015-2024  Urs Fleisch
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

#include "qmlcommandplugin.h"
#include <QDir>
#if !defined NDEBUG && !defined QT_QML_DEBUG
#define QT_QML_DEBUG
#endif
#include <QQuickView>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QTimer>
#include "kid3application.h"

/**
 * Constructor.
 *
 * @param parent parent object
 */
QmlCommandPlugin::QmlCommandPlugin(QObject* parent) : QObject(parent),
  m_app(nullptr), m_qmlView(nullptr), m_qmlEngine(nullptr), m_showOutput(false)
{
  setObjectName(QLatin1String("QmlCommand"));
}

/**
 * Get keys of available user commands.
 * @return list of keys, ["qml", "qmlview"].
 */
QStringList QmlCommandPlugin::userCommandKeys() const
{
  return {QLatin1String("qml"), QLatin1String("qmlview")};
}

/**
 * Initialize processor.
 * This method must be invoked before the first call to startUserCommand()
 * to set the application context.
 * @param app application context
 */
void QmlCommandPlugin::initialize(Kid3Application* app)
{
  m_app = app;
}

/**
 * Cleanup processor.
 * This method must be invoked to close and delete the GUI resources.
 */
void QmlCommandPlugin::cleanup()
{
  if (m_qmlView) {
    m_qmlView->close();
  }
  delete m_qmlView;
  m_qmlView = nullptr;
  // If m_qmlEngine is deleted, kid3-cli segfaults the second time "execute @qml"
  // is used in interactive mode. The UI application will crash if both
  // m_qmlEngine and m_qmlView have been constructed, deleting both
  // will result in a crash upon termination (Bug 457655). This happens when
  // using both a @qml user action (e.g. TitleCase.qml) and a @qmlview
  // user action (QmlConsole.qml). This also happens if m_app is set as a parent
  // of m_qmlEngine and it is deleted when m_app is deleted. Therefore,
  // m_qmlEngine is not deleted.
  if (s_messageHandlerInstance == this) {
    s_messageHandlerInstance = nullptr;
  }
}

/**
 * Start a QML script.
 * @param key user command name, "qml" or "qmlview"
 * @param arguments arguments to pass to script
 * @param showOutput true to enable output in output viewer, using signal
 *                   commandOutput()
 * @param vars variables to make available in command
 * @return true if command is started.
 */
bool QmlCommandPlugin::startUserCommand(
    const QString& key, const QStringList& arguments, bool showOutput,
    const QVariantMap& vars)
{
  if (!arguments.isEmpty()) {
    if (key == QLatin1String("qmlview")) {
      m_showOutput = showOutput;
      if (!m_qmlView) {
        m_qmlView = new QQuickView;
        m_qmlView->setResizeMode(QQuickView::SizeRootObjectToView);
        setupQmlEngine(m_qmlView->engine());
        // New style functor based connection is not possible because
        // QQuickCloseEvent is not public (QTBUG-36453, QTBUG-55722).
        connect(m_qmlView, SIGNAL(closing(QQuickCloseEvent*)), // clazy:exclude=old-style-connect
                this, SLOT(onQmlViewClosing()));
        connect(m_qmlView->engine(), &QQmlEngine::quit,
                this, &QmlCommandPlugin::onQmlViewFinished, Qt::QueuedConnection);
      }
      m_qmlView->engine()->rootContext()->setContextProperty(
            QLatin1String("args"), arguments);
      m_qmlEngine->rootContext()->setContextProperty(
            QLatin1String("vars"), vars);
      onEngineReady();
      m_qmlView->setSource(QUrl::fromLocalFile(arguments.first()));
      if (m_qmlView->status() == QQuickView::Ready) {
        m_qmlView->show();
      } else {
        // Probably an error.
        if (m_showOutput && m_qmlView->status() == QQuickView::Error) {
          const auto errs = m_qmlView->errors();
          for (const QQmlError& err : errs) {
            emit commandOutput(err.toString());
          }
        }
        m_qmlView->engine()->clearComponentCache();
        onEngineFinished();
      }
      return true;
    }
    if (key == QLatin1String("qml")) {
      m_showOutput = showOutput;
      if (!m_qmlEngine) {
        m_qmlEngine = new QQmlEngine;
        connect(m_qmlEngine, &QQmlEngine::quit, this, &QmlCommandPlugin::onQmlEngineQuit);
        setupQmlEngine(m_qmlEngine);
      }
      m_qmlEngine->rootContext()->setContextProperty(QLatin1String("args"),
                                                     arguments);
      m_qmlEngine->rootContext()->setContextProperty(QLatin1String("vars"),
                                                     vars);
      if (QQmlComponent component(m_qmlEngine, arguments.first());
        component.status() == QQmlComponent::Ready) {
        onEngineReady();
        component.create();
      } else {
        // Probably an error.
        if (m_showOutput && component.isError()) {
          const auto errs = component.errors();
          for (const QQmlError& err : errs) {
            emit commandOutput(err.toString());
          }
        }
        m_qmlEngine->clearComponentCache();
        onEngineFinished();
      }
      return true;
    }
  }
  return false;
}

/**
 * Set import path and app property in QML engine.
 * @param engine QML engine
 */
void QmlCommandPlugin::setupQmlEngine(QQmlEngine* engine)
{
#ifdef Q_OS_MAC
  // Folders containing a dot (like QtQuick.2) will cause Apple's code signing
  // to fail. On macOS, the QML plugins are therefore in Resorces/qml/imports.
  const QString qmlImportsRelativeToPlugins =
      QLatin1String("../Resources/qml/imports");
#else
  const QString qmlImportsRelativeToPlugins = QLatin1String("imports");
#endif
  if (QDir pluginsDir;
      Kid3Application::findPluginsDirectory(pluginsDir) &&
      pluginsDir.cd(qmlImportsRelativeToPlugins)) {
    engine->addImportPath(pluginsDir.absolutePath());
  }
  engine->rootContext()->setContextProperty(QLatin1String("app"), m_app);
  connect(engine, &QQmlEngine::warnings,
          this, &QmlCommandPlugin::onEngineError,
          Qt::UniqueConnection);
}

/**
 * Return object which emits commandOutput() signal.
 * @return this.
 */
QObject* QmlCommandPlugin::qobject()
{
  return this;
}

/**
 * Called when an error is reported by the QML engine.
 */
void QmlCommandPlugin::onEngineError(const QList<QQmlError>& errors)
{
  if (auto engine = qobject_cast<QQmlEngine*>(sender())) {
    for (const QQmlError& err : errors) {
      emit commandOutput(err.toString());
    }
    engine->clearComponentCache();
    onEngineFinished();
  }
}

/**
 * Called when the QML view is closing.
 */
void QmlCommandPlugin::onQmlViewClosing()
{
  if (auto view = qobject_cast<QQuickView*>(sender())) {
    // This will invoke destruction of the currently loaded QML code.
    view->setSource(QUrl());
    view->engine()->clearComponentCache();
    onEngineFinished();
  }
}

/**
 * Called when Qt.quit() is called from the QML code in the QQuickView.
 */
void QmlCommandPlugin::onQmlViewFinished()
{
  if (m_qmlView) {
    m_qmlView->close();
    // Unfortunately, calling close() on the QQuickView will not give a
    // QEvent::Close in an installed event filter, there is no closeEvent(),
    // closing() is not signalled. What remains is the hard way.
    // Calling m_qmlView->deleteLater() will cause a crash when the QML console
    // is started, a command executed (e.g. app.nextFile()), then .quit and
    // then a qml script is started.
    m_qmlView = nullptr;
    QTimer::singleShot(0, this, &QmlCommandPlugin::onEngineFinished);
  }
}

/**
 * Called when Qt.quit() is called from the QML code in the core engine.
 */
void QmlCommandPlugin::onQmlEngineQuit()
{
  if (m_qmlEngine) {
    m_qmlEngine->clearComponentCache();
  }
  onEngineFinished();
}

/**
 * Restore default message handler after QML code is terminated.
 */
void QmlCommandPlugin::onEngineFinished()
{
  if (m_showOutput) {
    qInstallMessageHandler(nullptr);
    s_messageHandlerInstance = nullptr;
  }
  QTimer::singleShot(0, this, [this] { emit finished(0); });
}

/**
 * Forward console output to output viewer while QML code is executed.
 */
void QmlCommandPlugin::onEngineReady()
{
  if (m_showOutput) {
    s_messageHandlerInstance = this;
    qInstallMessageHandler(messageHandler);
  }
}

/** Instance of QmlCommandPlugin running and generating messages. */
QmlCommandPlugin* QmlCommandPlugin::s_messageHandlerInstance = nullptr;

/**
 * Message handler emitting commandOutput().
 */
void QmlCommandPlugin::messageHandler(QtMsgType, const QMessageLogContext&, const QString& msg)
{
  if (s_messageHandlerInstance) {
    emit s_messageHandlerInstance->commandOutput(msg);
  }
}
