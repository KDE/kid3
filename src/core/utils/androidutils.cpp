/**
 * \file androidutils.cpp
 * Platform utility functions for Android.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 27 Feb 2019
 *
 * Copyright (C) 2019  Urs Fleisch
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

#include "androidutils.h"
#ifdef Q_OS_ANDROID
#include <jni.h>
#if QT_VERSION >= 0x060000
#include <QCoreApplication>
#include <QJniObject>
#else
#include <QtAndroid>
#include <QAndroidJniObject>
#endif
#endif

AndroidUtils* AndroidUtils::s_self = nullptr;

AndroidUtils::AndroidUtils(QObject* parent) : QObject(parent)
{
  Q_ASSERT_X(!s_self, "AndroidUtils", "there should be only one instance");
  s_self = this;
}

void AndroidUtils::checkPendingIntents()
{
#ifdef Q_OS_ANDROID
#if QT_VERSION >= 0x060000
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
#else
  QAndroidJniObject activity = QtAndroid::androidActivity();
#endif
  if (activity.isValid()) {
    activity.callMethod<void>("checkPendingIntents");
  }
#endif
}

void AndroidUtils::emitFilePathReceived(const QString& path)
{
  emit filePathReceived(path);
}


#ifdef Q_OS_ANDROID
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_net_sourceforge_kid3_Kid3Activity_setFilePathFromIntent(
    JNIEnv* env, jobject obj, jstring path)
{
  Q_UNUSED(obj)
  const char* pathStr = env->GetStringUTFChars(path, NULL);
  if (AndroidUtils* utils = AndroidUtils::instance()) {
    utils->emitFilePathReceived(QString::fromUtf8(pathStr));
  }
  env->ReleaseStringUTFChars(path, pathStr);
}

#ifdef __cplusplus
}
#endif
#endif
