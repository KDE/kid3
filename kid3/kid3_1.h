/**
 * \file kid3.h
 * Kid3 application.
 * kid3.h is generated from kid3_1.h and kid3_2.h. Do not edit kid3.h.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "autoconf.h"
#include "filelist.h"
#include "standardtags.h"
#include "framelist.h"

// forward declaration of the Kid3 classes
#ifdef CONFIG_USE_KDE
#include <kmainwindow.h>
class KAction;
class KRecentFilesAction;
class KToggleAction;
class KURL;
#else
#include <qmainwindow.h>
class QAction;
#endif
class id3Form;
class Mp3File;
