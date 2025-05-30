// Qt
#include <QtGlobal>
#include <QGuiApplication>

// MythTV
#include "libmythbase/mythconfig.h"

#include "mythmainwindow.h"
#include "mythscreensaver.h"

#if CONFIG_DRM
#include "platforms/mythscreensaverdrm.h"
#endif

#if CONFIG_QTDBUS
#include "platforms/mythscreensaverdbus.h"
#endif

#ifdef Q_OS_DARWIN
#include "platforms/mythscreensaverosx.h"
#endif

#ifdef Q_OS_ANDROID
#include "platforms/mythscreensaverandroid.h"
#endif

#if CONFIG_WAYLANDEXTRAS
#include "platforms/mythscreensaverwayland.h"
#endif

#if CONFIG_X11
#include "platforms/mythscreensaverx11.h"
#include "platforms/mythxdisplay.h"
#endif

/*! \class MythScreenSaverControl
 *
 * \note This constructor is called from the MythMainWindow constructor. Do NOT
 * use the MythMainWindow object here (or in any MythScreenSaver constructor) as
 * it is not complete. Instead listen for the MythMainWindow::signalWindowReady signal.
*/
MythScreenSaverControl::MythScreenSaverControl([[maybe_unused]] MythMainWindow* MainWin,
                                               [[maybe_unused]] MythDisplay* mDisplay)
{
#if CONFIG_QTDBUS
    m_screenSavers.push_back(new MythScreenSaverDBus(this));
#endif
#if CONFIG_X11
    MythXDisplay* display = MythXDisplay::OpenMythXDisplay(false);
    if (display)
    {
        m_screenSavers.push_back(new MythScreenSaverX11(this));
        delete display;
    }
#elif defined(Q_OS_DARWIN)
    m_screenSavers.push_back(new MythScreenSaverOSX(this));
#endif
#if defined(ANDROID)
    m_screenSavers.push_back(new MythScreenSaverAndroid(this));
#endif
#if CONFIG_DRM
    MythScreenSaverDRM* drmsaver = MythScreenSaverDRM::Create(this, mDisplay);
    if (drmsaver)
        m_screenSavers.push_back(drmsaver);
#endif
#if CONFIG_WAYLANDEXTRAS
    if (QGuiApplication::platformName().toLower().contains("wayland"))
        m_screenSavers.push_back(new MythScreenSaverWayland(this, MainWin));
#endif

    for (auto * screensaver : m_screenSavers)
    {
        connect(this, &MythScreenSaverControl::Disable, screensaver, &MythScreenSaver::Disable);
        connect(this, &MythScreenSaverControl::Reset,   screensaver, &MythScreenSaver::Reset);
        connect(this, &MythScreenSaverControl::Restore, screensaver, &MythScreenSaver::Restore);
    }
}

bool MythScreenSaverControl::Asleep()
{
    for (auto * screensaver : m_screenSavers)
        if (screensaver->Asleep())
            return true;
    return false;
}
