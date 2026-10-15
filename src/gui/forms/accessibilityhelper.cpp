/**
 * \file accessibilityhelper.cpp
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

#include "accessibilityhelper.h"

#if defined Q_OS_MACOS && QT_VERSION >= 0x060800 && !defined QT_NO_ACCESSIBILITY

#include <QAbstractItemView>
#include <QAccessible>
#include <QAccessibleWidget>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QFocusEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QMenu>
#include <QStringList>
#include <QTableView>
#include <QTimer>

namespace {

/**
 * Accessible interface of the viewport of an item view.
 *
 * Editors are children of the viewport. The macOS accessibility bridge
 * treats the children of a table as rows and cells, so the parent chain
 * of an editor is broken at the table and VoiceOver cannot reach it.
 * Reporting the parent of the view as parent of the viewport fixes this.
 */
class ViewportAccessible : public QAccessibleWidget {
public:
  explicit ViewportAccessible(QWidget* viewport)
    : QAccessibleWidget(viewport) {}

  QAccessibleInterface* parent() const override {
    if (QWidget* view = widget()->parentWidget()) {
      QWidget* viewParent = view->parentWidget();
      return QAccessible::queryAccessibleInterface(
            viewParent ? static_cast<QObject*>(viewParent)
                       : static_cast<QObject*>(qApp));
    }
    return QAccessibleWidget::parent();
  }
};

QAccessibleInterface* viewportAccessibleFactory(const QString& className,
                                                QObject* object)
{
  if (className == QLatin1String("QWidget")) {
    if (auto widget = qobject_cast<QWidget*>(object)) {
      if (auto view = qobject_cast<QAbstractItemView*>(widget->parentWidget());
          view && view->viewport() == widget &&
          !qobject_cast<QHeaderView*>(view)) {
        return new ViewportAccessible(widget);
      }
    }
  }
  return nullptr;
}

QString itemText(const QModelIndex& index)
{
  QString text = index.data(Qt::AccessibleTextRole).toString();
  if (text.isEmpty()) {
    text = index.data(Qt::DisplayRole).toString();
  }
  return text;
}

QString describeItem(const QAbstractItemView* view, const QModelIndex& index)
{
  QStringList parts;
  if (qobject_cast<const QTableView*>(view)) {
    // Use the column header or else the first cell of the row as label.
    QString label = index.model()->headerData(
          index.column(), Qt::Horizontal, Qt::DisplayRole).toString();
    if (label.isEmpty() && index.column() > 0) {
      label = itemText(index.sibling(index.row(), 0));
    }
    if (!label.isEmpty()) {
      parts.append(label);
    }
  } else if (index.column() > 0) {
    // In trees, name the item and the column of other cells.
    for (const QString& label :
         {itemText(index.sibling(index.row(), 0)),
          index.model()->headerData(index.column(), Qt::Horizontal,
                                    Qt::DisplayRole).toString()}) {
      if (!label.isEmpty()) {
        parts.append(label);
      }
    }
  }
  if (QString text = itemText(index); !text.isEmpty()) {
    parts.append(text);
  }
  if (QVariant checkState = index.data(Qt::CheckStateRole);
      checkState.isValid()) {
    switch (checkState.toInt()) {
    case Qt::Checked:
      parts.append(QCoreApplication::translate("AccessibilityHelper",
                                               "checked"));
      break;
    case Qt::PartiallyChecked:
      parts.append(QCoreApplication::translate("AccessibilityHelper",
                                               "partially checked"));
      break;
    default:
      parts.append(QCoreApplication::translate("AccessibilityHelper",
                                               "not checked"));
      break;
    }
  }
  if (QString description = index.data(Qt::AccessibleDescriptionRole).toString();
      !description.isEmpty()) {
    parts.append(description);
  }
  return parts.join(QLatin1String(", "));
}

/**
 * Event filter for an item view.
 *
 * When an item view gets the focus, Qt notifies the focus change of the
 * view after the one of its current item, so VoiceOver does not move to the
 * item and keeps describing the previously focused widget. Notifying the
 * current item again afterwards behaves as when the item is changed with
 * the cursor keys.
 *
 * The items do not support the accessibility action to show the context
 * menu, so VoiceOver cannot open it. The keyboard shortcut of macOS to show
 * the context menu (Control-Return) would start editing the current item,
 * so it is handled here to open the context menu of the current item.
 */
class ItemViewWatcher : public QObject {
public:
  explicit ItemViewWatcher(QAbstractItemView* view) : QObject(view), m_view(view) {}

  bool eventFilter(QObject* watched, QEvent* event) override {
    if (watched == m_view && event->type() == QEvent::KeyPress) {
      // The Control key is reported as Meta modifier on macOS.
      if (const auto keyEvent = static_cast<QKeyEvent*>(event);
          (keyEvent->key() == Qt::Key_Return ||
           keyEvent->key() == Qt::Key_Enter) &&
          (keyEvent->modifiers() & ~Qt::KeypadModifier) == Qt::MetaModifier) {
        return showContextMenu();
      }
    } else if (watched == m_view && event->type() == QEvent::FocusIn &&
        QAccessible::isActive()) {
      if (Qt::FocusReason reason = static_cast<QFocusEvent*>(event)->reason();
          reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason ||
          reason == Qt::ShortcutFocusReason) {
        // Wait until the focus event of the view has been sent by Qt.
        QTimer::singleShot(0, this, [this] {
          notifyCurrentItemFocus();
        });
      }
    }
    return false;
  }

private:
  bool showContextMenu() {
    // Mouse context menu events are sent to the viewport, the item views
    // of Kid3 expect the position in viewport coordinates.
    QWidget* viewport = m_view->viewport();
    QModelIndex current = m_view->currentIndex();
    QRect rect = current.isValid() ? m_view->visualRect(current) : QRect();
    if (!rect.isValid() || !viewport->rect().intersects(rect)) {
      rect = QRect(QPoint(0, 0), QSize(1, 1));
    }
    const QPoint pos = rect.center();
    QContextMenuEvent contextMenuEvent(QContextMenuEvent::Keyboard, pos,
                                       viewport->mapToGlobal(pos));
    QCoreApplication::sendEvent(viewport, &contextMenuEvent);
    return contextMenuEvent.isAccepted();
  }

  void notifyCurrentItemFocus() {
    QModelIndex current = m_view->currentIndex();
    if (!current.isValid() || !m_view->hasFocus())
      return;
    if (QAccessibleInterface* iface =
        QAccessible::queryAccessibleInterface(m_view)) {
      if (QAccessibleInterface* child = iface->focusChild()) {
        if (int childIndex = iface->indexOfChild(child); childIndex >= 0) {
          QAccessibleEvent focusEvent(m_view, QAccessible::Focus);
          focusEvent.setChild(childIndex);
          QAccessible::updateAccessibility(&focusEvent);
        }
      }
    }
    AccessibilityHelper::announceItem(m_view, current);
  }

  QAbstractItemView* m_view;
};

}

void AccessibilityHelper::setupItemView(QAbstractItemView* view)
{
  if (!view)
    return;
  QAccessible::installFactory(viewportAccessibleFactory);
  // Replace the connections of a previous call, e.g. for a new selection model.
  const QString contextName = QLatin1String("kid3AccessibilityAnnouncer");
  while (QObject* old = view->findChild<QObject*>(
           contextName, Qt::FindDirectChildrenOnly)) {
    delete old;
  }
  auto viewWatcher = new ItemViewWatcher(view);
  viewWatcher->setObjectName(contextName);
  view->installEventFilter(viewWatcher);
  if (QItemSelectionModel* selectionModel = view->selectionModel()) {
    auto context = new QObject(view);
    context->setObjectName(contextName);
    QObject::connect(
          selectionModel, &QItemSelectionModel::currentChanged, context,
          [view](const QModelIndex& current) {
      // Only announce changes made by the user while the view is in use.
      if (!current.isValid() || !QAccessible::isActive())
        return;
      // The focus widget of a popup, e.g. of a combo box, is not reliable.
      const bool inPopup = view->isVisible() &&
          view->window()->windowType() == Qt::Popup;
      QWidget* focusWidget = QApplication::focusWidget();
      if (!inPopup && (!focusWidget || (focusWidget != view &&
                                        !view->isAncestorOf(focusWidget))))
        return;
      if (QString message = describeItem(view, current); !message.isEmpty()) {
        QAccessibleAnnouncementEvent event(view, message);
        QAccessible::updateAccessibility(&event);
      }
    });
  }
  if (QAbstractItemModel* model = view->model()) {
    auto context = new QObject(view);
    context->setObjectName(contextName);
    QObject::connect(
          model, &QAbstractItemModel::dataChanged, context,
          [view](const QModelIndex& topLeft, const QModelIndex& bottomRight,
                 const QList<int>& roles) {
      // Announce the new state when the user toggles the current item.
      if (!roles.contains(Qt::CheckStateRole) || !QAccessible::isActive() ||
          !view->hasFocus())
        return;
      if (QModelIndex current = view->currentIndex();
          current.isValid() && current.parent() == topLeft.parent() &&
          current.row() >= topLeft.row() &&
          current.row() <= bottomRight.row() &&
          current.column() >= topLeft.column() &&
          current.column() <= bottomRight.column()) {
        AccessibilityHelper::announceItem(view, current);
      }
    });
  }
}

namespace {

/**
 * Event filter for a combo box and its popup.
 *
 * The popup of a combo box is a separate window which is not followed by
 * VoiceOver, so its items are announced like the items of other views.
 * When the current item of a closed combo box is changed with the cursor
 * keys, VoiceOver ignores the value change, so the new item is announced.
 */
class ComboBoxWatcher : public QObject {
public:
  explicit ComboBoxWatcher(QComboBox* comboBox)
    : QObject(comboBox), m_comboBox(comboBox) {}

  /**
   * Check if an object is the popup container of a combo box.
   * @param obj object
   * @return true if @a obj is a popup container.
   */
  static bool isPopupContainer(const QObject* obj) {
    return obj->isWidgetType() && obj->inherits("QComboBoxPrivateContainer");
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if (watched == m_comboBox) {
      if (event->type() == QEvent::ChildPolished) {
        // The popup container is created when it is needed and is polished
        // before it is shown for the first time.
        if (QObject* child = static_cast<QChildEvent*>(event)->child();
            isPopupContainer(child)) {
          child->installEventFilter(this);
        }
      } else if (event->type() == QEvent::KeyPress &&
                 QAccessible::isActive() && m_comboBox->hasFocus()) {
        switch (static_cast<QKeyEvent*>(event)->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Home:
        case Qt::Key_End: {
          // Check after the key has been handled by the combo box.
          const int oldIndex = m_comboBox->currentIndex();
          QTimer::singleShot(0, m_comboBox, [comboBox = m_comboBox, oldIndex] {
            if (comboBox->currentIndex() != oldIndex &&
                comboBox->hasFocus() && !comboBox->view()->isVisible()) {
              AccessibilityHelper::announce(comboBox, comboBox->currentText());
            }
          });
          break;
        }
        default:
          break;
        }
      }
    } else if (event->type() == QEvent::Show && QAccessible::isActive()) {
      // The popup container is shown.
      QAbstractItemView* view = m_comboBox->view();
      AccessibilityHelper::setupItemView(view);
      // Announce the item which is current when the popup opens.
      QTimer::singleShot(0, view, [view] {
        if (QModelIndex index = view->currentIndex();
            index.isValid() && view->isVisible()) {
          AccessibilityHelper::announceItem(view, index);
        }
      });
    }
    return false;
  }

private:
  QComboBox* m_comboBox;
};

}

void AccessibilityHelper::setupComboBox(QComboBox* comboBox)
{
  const QString watcherName = QLatin1String("kid3AccessibilityComboBoxWatcher");
  if (!comboBox ||
      comboBox->findChild<QObject*>(watcherName, Qt::FindDirectChildrenOnly))
    return;
  auto watcher = new ComboBoxWatcher(comboBox);
  watcher->setObjectName(watcherName);
  comboBox->installEventFilter(watcher);
  // Normally, the popup container does not exist yet and the watcher is
  // installed on it when it is polished.
  const QObjectList children = comboBox->children();
  for (QObject* child : children) {
    if (ComboBoxWatcher::isPopupContainer(child)) {
      child->installEventFilter(watcher);
    }
  }
}

namespace {

QString describeMenu(const QMenu* menu)
{
  QStringList parts;
  QString name = menu->accessibleName();
  if (name.isEmpty()) {
    name = menu->title();
  }
  if (!name.isEmpty()) {
    parts.append(name.remove(QLatin1Char('&')));
  }
  int numItems = 0;
  const QList<QAction*> actions = menu->actions();
  for (const QAction* action : actions) {
    if (action->isVisible() && !action->isSeparator()) {
      ++numItems;
    }
  }
  parts.append(QCoreApplication::translate("AccessibilityHelper", "menu"));
  parts.append(QCoreApplication::translate("AccessibilityHelper", "%n items",
                                           nullptr, numItems));
  return parts.join(QLatin1String(", "));
}

QString describeAction(const QAction* action)
{
  QStringList parts;
  parts.append(action->text().remove(QLatin1Char('&')));
  if (action->isCheckable()) {
    parts.append(action->isChecked()
                 ? QCoreApplication::translate("AccessibilityHelper",
                                               "checked")
                 : QCoreApplication::translate("AccessibilityHelper",
                                               "not checked"));
  }
  if (action->menu()) {
    parts.append(QCoreApplication::translate("AccessibilityHelper",
                                             "submenu"));
  }
  if (!action->isEnabled()) {
    parts.append(QCoreApplication::translate("AccessibilityHelper", "dimmed"));
  }
  return parts.join(QLatin1String(", "));
}

/**
 * Announce a menu and its submenus when they are opened.
 * @param menu menu
 * @return context object of the connections, nullptr if already set up.
 */
QObject* setupMenuAnnouncement(QMenu* menu)
{
  const QString contextName = QLatin1String("kid3AccessibilityMenuAnnouncer");
  if (menu->findChild<QObject*>(contextName, Qt::FindDirectChildrenOnly))
    return nullptr;
  auto context = new QObject(menu);
  context->setObjectName(contextName);
  QObject::connect(menu, &QMenu::aboutToShow, context, [menu] {
    // Submenus can be added after the setup, so they are set up here.
    const QList<QAction*> actions = menu->actions();
    for (const QAction* action : actions) {
      if (QMenu* subMenu = action->menu()) {
        setupMenuAnnouncement(subMenu);
      }
    }
    if (!QAccessible::isActive())
      return;
    // Announce the menu after it has been shown.
    QTimer::singleShot(0, menu, [menu] {
      if (menu->isVisible()) {
        AccessibilityHelper::announce(menu, describeMenu(menu));
      }
    });
  });
  return context;
}

}

void AccessibilityHelper::setupMenu(QMenu* menu)
{
  QObject* context = menu ? setupMenuAnnouncement(menu) : nullptr;
  if (!context)
    return;
  // QMenu::hovered() is also emitted by the parent menus for the actions
  // of submenus, so the items of the submenus are announced too.
  QObject::connect(menu, &QMenu::hovered, context, [menu](QAction* action) {
    // The active action is changed with the cursor keys or the mouse.
    if (action && QAccessible::isActive() && menu->isVisible()) {
      AccessibilityHelper::announce(menu, describeAction(action));
    }
  });
}

void AccessibilityHelper::announceItem(QAbstractItemView* view,
                                       const QModelIndex& index)
{
  if (QAccessible::isActive()) {
    announce(view, describeItem(view, index));
  }
}

void AccessibilityHelper::announce(QWidget* widget, const QString& message)
{
  if (widget && !message.isEmpty() && QAccessible::isActive()) {
    QAccessibleAnnouncementEvent event(widget, message);
    QAccessible::updateAccessibility(&event);
  }
}

#endif
