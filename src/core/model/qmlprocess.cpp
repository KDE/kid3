/**
 * \file qmlprocess.cpp
 * Starter for QML scripts.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
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

#include "qmlprocess.h"
#include <QDir>
#if QT_VERSION >= 0x050000
#include <QQuickView>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QTimer>
#else
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeComponent>
#include <QDeclarativeItem>
#include <QCloseEvent>
#endif
#include "kid3application.h"

#if QT_VERSION < 0x050000
//! @cond
#define QQmlEngine QDeclarativeEngine
#define QQuickView QmlView
#define QQmlComponent QDeclarativeComponent
#define QQmlError QDeclarativeError
//! @endcond
#endif

namespace {

/**
 * Set import path and app property in QML engine.
 * @param engine QML engine
 * @param app application context
 */
void setupQmlEngine(QQmlEngine* engine, Kid3Application* app)
{
  QDir pluginsDir;
  if (Kid3Application::findPluginsDirectory(pluginsDir) &&
      pluginsDir.cd(QLatin1String("imports"))) {
    engine->addImportPath(pluginsDir.absolutePath());
  }
  engine->rootContext()->setContextProperty(QLatin1String("app"), app);
}

}

/**
 * Constructor.
 *
 * @param app application context
 * @param parent parent object
 */
QmlProcess::QmlProcess(Kid3Application* app, QObject* parent) : QObject(parent),
  m_app(app), m_qmlView(0), m_qmlEngine(0), m_showOutput(false)
{
}

/**
 * Destructor.
 */
QmlProcess::~QmlProcess()
{
  if (m_qmlView) {
    m_qmlView->close();
  }
  delete m_qmlView;
  if (s_messageHandlerInstance == this) {
    s_messageHandlerInstance = 0;
  }
}

/**
 * Start a QML script.
 *
 * @param program virtual program, e.g. "qmlview"
 * @param arguments arguments to pass to script
 * @param showOutput true to enable output in output viewer
 * @return true if program and arguments are suitable for QML script.
 */
bool QmlProcess::startQml(const QString& program,
                          const QStringList& arguments, bool showOutput)
{
  if (!arguments.isEmpty()) {
    if (program == QLatin1String("qmlview")) {
      m_showOutput = showOutput;
      if (!m_qmlView) {
        m_qmlView = new QQuickView;
        m_qmlView->setResizeMode(QQuickView::SizeRootObjectToView);
        setupQmlEngine(m_qmlView->engine(), m_app);
        connect(m_qmlView, SIGNAL(closing(QQuickCloseEvent*)),
                this, SLOT(onQmlViewClosing()));
        connect(m_qmlView->engine(), SIGNAL(quit()),
                this, SLOT(onQmlViewFinished()), Qt::QueuedConnection);
      }
      m_qmlView->engine()->rootContext()->setContextProperty(
            QLatin1String("args"), arguments);
      onEngineReady();
      m_qmlView->setSource(QUrl::fromLocalFile(arguments.first()));
      if (m_qmlView->status() == QQuickView::Ready) {
        m_qmlView->show();
      } else {
        // Probably an error.
        if (m_showOutput && m_qmlView->status() == QQuickView::Error) {
          foreach (const QQmlError& err, m_qmlView->errors()) {
            emit qmlOutput(err.toString());
          }
        }
        m_qmlView->engine()->clearComponentCache();
        onEngineFinished();
      }
      return true;
    } else if (program == QLatin1String("qml")) {
      m_showOutput = showOutput;
      if (!m_qmlEngine) {
        m_qmlEngine = new QQmlEngine(this);
        connect(m_qmlEngine, SIGNAL(quit()), this, SLOT(onQmlEngineQuit()));
        setupQmlEngine(m_qmlEngine, m_app);
      }
      m_qmlEngine->rootContext()->setContextProperty(QLatin1String("args"),
                                                     arguments);
      QQmlComponent component(m_qmlEngine, arguments.first());
      if (component.status() == QQmlComponent::Ready) {
        onEngineReady();
        component.create();
      } else {
        // Probably an error.
        if (m_showOutput && component.isError()) {
          foreach (const QQmlError& err, component.errors()) {
            emit qmlOutput(err.toString());
          }
        }
        m_qmlEngine->clearComponentCache();
      }
      return true;
    }
  }
  return false;
}

/**
 * Called when the QML view is closing.
 */
void QmlProcess::onQmlViewClosing()
{
  if (QQuickView* view = qobject_cast<QQuickView*>(sender())) {
    // This will invoke destruction of the currently loaded QML code.
    view->setSource(QUrl());
    view->engine()->clearComponentCache();
    onEngineFinished();
  }
}

/**
 * Called when Qt.quit() is called from the QML code in the QQuickView.
 */
void QmlProcess::onQmlViewFinished()
{
  if (m_qmlView) {
#if QT_VERSION >= 0x050000
    // Unfortunately, calling close() on the QQuickView will not give a
    // QEvent::Close in an installed event filter, there is no closeEvent(),
    // closing() is not signalled. What remains is the hard way.
    m_qmlView->deleteLater();
    m_qmlView = 0;
    QTimer::singleShot(0, this, SLOT(onEngineFinished()));
#else
    m_qmlView->close();
#endif
  }
}

/**
 * Called when Qt.quit() is called from the QML code in the core engine.
 */
void QmlProcess::onQmlEngineQuit()
{
  if (m_qmlEngine) {
    m_qmlEngine->clearComponentCache();
  }
  onEngineFinished();
}

/**
 * Restore default message handler after QML code is terminated.
 */
void QmlProcess::onEngineFinished()
{
  if (m_showOutput) {
#if QT_VERSION >= 0x050000
    qInstallMessageHandler(0);
#else
    qInstallMsgHandler(0);
#endif
    s_messageHandlerInstance = 0;
  }
}

/**
 * Forward console output to output viewer while QML code is executed.
 */
void QmlProcess::onEngineReady()
{
  if (m_showOutput) {
    s_messageHandlerInstance = this;
#if QT_VERSION >= 0x050000
    qInstallMessageHandler(messageHandler);
#else
    qInstallMsgHandler(messageHandler);
#endif
  }
}

/** Instance of QmlProcess running and generating messages. */
QmlProcess* QmlProcess::s_messageHandlerInstance = 0;

/**
 * Message handler emitting qmlOutput().
 */
#if QT_VERSION >= 0x050000
void QmlProcess::messageHandler(QtMsgType, const QMessageLogContext&, const QString& msg)
{
  if (s_messageHandlerInstance) {
    emit s_messageHandlerInstance->qmlOutput(msg);
  }
}
#else
void QmlProcess::messageHandler(QtMsgType, const char* msg)
{
  if (s_messageHandlerInstance) {
    emit s_messageHandlerInstance->qmlOutput(QString::fromUtf8(msg));
  }
}
#endif


#if QT_VERSION < 0x050000
/**
 * Constructor.
 * @param parent parent widget
 */
QmlView::QmlView(QWidget* parent) : QDeclarativeView(parent)
{
}

/**
 * Destructor.
 */
QmlView::~QmlView()
{
}

/**
 * Handle close event.
 * @param ev close event
 */
void QmlView::closeEvent(QCloseEvent* ev)
{
  ev->accept();
  emit closing(0);
}
#endif
