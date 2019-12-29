//Qt
#include <QTimer>
#include <QThread>
#include <QApplication>
#include <QElapsedTimer>
#include <QWindow>

// MythTV
#include <mythlogging.h>
#include "compat.h"
#include "mythcorecontext.h"
#include "mythdisplay.h"
#include "mythmainwindow.h"

#ifdef Q_OS_ANDROID
#include "mythdisplayandroid.h"
#endif
#if defined(Q_OS_MAC)
#include "mythdisplayosx.h"
#endif
#ifdef USING_X11
#include "mythdisplayx11.h"
#endif
#ifdef USING_DRM
#include "mythdisplaydrm.h"
#endif
#if defined(Q_OS_WIN)
#include "mythdisplaywindows.h"
#endif

#define LOC QString("Display: ")

/*! \class MythDisplay
 *
 * MythDisplay is a reference counted, singleton class.
 *
 * Retrieve a reference to the MythDisplay object by calling AcquireRelease().
 *
 * A valid pointer is always returned.
 *
 * Release the reference by calling AcquireRelease(false).
 *
 * \note There is no locking in MythDisplay. It should never be used from anywhere
 * other than the main/UI thread.
 *
 * \bug When using Xinerama/virtual screens, we get the correct screen at startup
 * and we successfully move to another screen at the first attempt. Subsequently
 * trying to move back to the original screen does not work. This appears to be
 * an issue with MythMainWindow relying on MythUIHelper for screen coordinates
 * that are not updated and/or subsequent window movements moving the window
 * into the wrong screen. Much of the MythUIHelper code needs to move into MythDisplay.
 *
 * \todo Complete handling of screen changes. We need to handle several cases.
 * Firstly, when the main window is moved (dragged) to a new screen. This generally
 * works but need to check if all display details are updated (VideoOutWindow is
 * currently hooked up to the CurrentScreenChanged signal - so video window sizing
 * should work without issue). Various other parameters need updating though e.g.
 * refresh rates, supported rates etc
 * Secondly, when a new screen is added and the user has configured a multiscreen
 * setup - in which case we might want to move the main window to the new screen.
 * There are various complications here. Currently switching windows is triggered
 * from the settings screens - which initiate a teardown of painter/render/UI etc.
 * We cannot do that from random parts of the UI or during video playback.
*/
MythDisplay* MythDisplay::AcquireRelease(bool Acquire)
{
    static QMutex s_lock(QMutex::Recursive);
    static MythDisplay* s_display = nullptr;

    QMutexLocker locker(&s_lock);

    if (Acquire)
    {
        if (s_display)
        {
            s_display->IncrRef();
        }
        else
        {
#ifdef USING_X11
            if (MythDisplayX11::IsAvailable())
                s_display = new MythDisplayX11();
#endif
#ifdef USING_DRM
#if QT_VERSION >= QT_VERSION_CHECK(5,9,0)
            // this will only work by validating the screen's serial number
            // - which is only available with Qt 5.9
            if (!s_display)
                s_display = new MythDisplayDRM();
#endif
#endif
#if defined(Q_OS_MAC)
            if (!s_display)
                s_display = new MythDisplayOSX();
#endif
#ifdef Q_OS_ANDROID
            if (!s_display)
                s_display = new MythDisplayAndroid();
#endif
#if defined(Q_OS_WIN)
            if (!s_display)
                s_display = new MythDisplayWindows();
#endif
            if (!s_display)
                s_display = new MythDisplay();
        }
    }
    else
    {
        if (s_display)
            if (s_display->DecrRef() == 0)
                s_display = nullptr;
    }
    return s_display;
}

MythDisplay::MythDisplay()
  : ReferenceCounter("Display")
{
    m_screen = GetDesiredScreen();
    DebugScreen(m_screen, "Using");
    if (m_screen)
        connect(m_screen, &QScreen::geometryChanged, this, &MythDisplay::GeometryChanged);

    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &MythDisplay::ScreenRemoved);
    connect(qGuiApp, &QGuiApplication::screenAdded, this, &MythDisplay::ScreenAdded);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &MythDisplay::PrimaryScreenChanged);
#endif
}

MythDisplay::~MythDisplay()
{
    LOG(VB_GENERAL, LOG_INFO, LOC + "Deleting");
}

void MythDisplay::SetWidget(QWidget *MainWindow)
{
    QWidget* old = m_widget;
    m_widget = MainWindow;

    if (!m_widget)
        return;
    if (m_widget != old)
        LOG(VB_GENERAL, LOG_INFO, LOC + "New main widget");

    QWindow* window = m_widget->windowHandle();
    if (window)
    {
        QScreen *desired = GetDesiredScreen();
        if (desired && (desired != window->screen()))
        {
            // If we have changed the video mode for the old screen then reset
            // it to the default/desktop mode
            SwitchToDesktop();
            // Ensure we completely re-initialise when the new screen is set
            m_initialised = false;

            DebugScreen(desired, "Moving to");
            // If this is a virtual desktop, move the window into the screen,
            // otherwise just set the screen - both of which should trigger a
            // screenChanged event.
            // TODO Confirm this check for non-virtual screens (OSX?)
            // TODO If the screens are non-virtual - can we actually safely move?
            // (SetWidget is only called from MythMainWindow before the render
            // device is created - so should be safe).
            if (desired->geometry() == desired->virtualGeometry())
                window->setScreen(desired);
            else
                m_widget->move(desired->geometry().topLeft());
        }
        connect(window, &QWindow::screenChanged, this, &MythDisplay::ScreenChanged);
        return;
    }

    LOG(VB_GENERAL, LOG_WARNING, LOC + "Widget does not have a window!");
}

int MythDisplay::GetScreenCount(void)
{
    return qGuiApp->screens().size();
}

double MythDisplay::GetPixelAspectRatio(void)
{
    if (m_physicalSize.width() > 0 && m_physicalSize.height() > 0 &&
        m_resolution.width() > 0 && m_resolution.height() > 0)
    {
        return (m_physicalSize.width() / static_cast<double>(m_resolution.width())) /
               (m_physicalSize.height() / static_cast<double>(m_resolution.height()));
    }
    return 1.0;
}

QSize MythDisplay::GetGUIResolution(void)
{
    return m_guiMode.Resolution();
}

/*! \brief Return a pointer to the screen to use.
 *
 * This function looks at the users screen preference, and will return
 * that screen if possible.  If not, i.e. the screen isn't plugged in,
 * then this function returns the system's primary screen.
 *
 * Note: There is no special case here for the case of MythTV spanning
 * all screens, as all screen have access to the virtual desktop
 * attributes.  The check for spanning screens must be made when the
 * screen size/geometry accessed, and the proper physical/virtual
 * size/geometry retrieved.
*/
QScreen* MythDisplay::GetCurrentScreen(void)
{
    return m_screen;
}

QScreen *MythDisplay::GetDesiredScreen(void)
{
    QScreen* newscreen = nullptr;

    // Lookup by name
    QString name = gCoreContext->GetSetting("XineramaScreen", nullptr);
    foreach (QScreen *screen, qGuiApp->screens())
    {
        if (!name.isEmpty() && name == screen->name())
        {
            LOG(VB_GENERAL, LOG_INFO, LOC + QString("Found screen '%1'").arg(name));
            newscreen = screen;
        }
    }

    // No name match.  These were previously numbers.
    if (!newscreen)
    {
        bool ok = false;
        int screen_num = name.toInt(&ok);
        QList<QScreen *>screens = qGuiApp->screens();
        if (ok && (screen_num >= 0) && (screen_num < screens.size()))
        {
            LOG(VB_GENERAL, LOG_INFO, LOC + QString("Found screen number %1 (%2)")
                .arg(name).arg(screens[screen_num]->name()));
            newscreen = screens[screen_num];
        }
    }

    // For anything else, return the primary screen.
    if (!newscreen)
    {
        QScreen *primary = qGuiApp->primaryScreen();
        if (name.isEmpty() && primary)
        {
            LOG(VB_GENERAL, LOG_INFO, LOC + QString("Defaulting to primary screen (%1)")
                .arg(primary->name()));
        }
        else if (name != "-1" && primary)
        {
            LOG(VB_GENERAL, LOG_INFO, LOC + QString("Screen '%1' not found, defaulting to primary screen (%2)")
                .arg(name).arg(primary->name()));
        }
        newscreen = primary;
    }

    return newscreen;
}

/*! \brief The actual screen in use has changed. We must use it.
*/
void MythDisplay::ScreenChanged(QScreen *qScreen)
{
    if (qScreen == m_screen)
        return;
    if (m_screen)
        disconnect(m_screen, nullptr, this, nullptr);
    DebugScreen(qScreen, "Changed to");
    m_screen = qScreen;
    connect(m_screen, &QScreen::geometryChanged, this, &MythDisplay::GeometryChanged);
    Initialise();
    emit CurrentScreenChanged(qScreen);
}

void MythDisplay::PrimaryScreenChanged(QScreen* qScreen)
{
    DebugScreen(qScreen, "New primary");
}

void MythDisplay::ScreenAdded(QScreen* qScreen)
{
    DebugScreen(qScreen, "New");
    emit ScreenCountChanged(qGuiApp->screens().size());
}

void MythDisplay::ScreenRemoved(QScreen* qScreen)
{
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Screen '%1' removed").arg(qScreen->name()));
    emit ScreenCountChanged(qGuiApp->screens().size());
}

void MythDisplay::GeometryChanged(const QRect &Geo)
{
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("New screen geometry: %1x%2+%3+%4")
        .arg(Geo.width()).arg(Geo.height()).arg(Geo.left()).arg(Geo.top()));
}

void MythDisplay::UpdateCurrentMode(void)
{
    // This is the final fallback when no other platform specifics are available
    // It is usually accurate apart from the refresh rate - which is often
    // rounded down.
    m_edid = MythEDID();
    QScreen *screen = GetCurrentScreen();
    if (!screen)
    {
        m_refreshRate = 60.0;
        m_physicalSize = QSize(160, 90);
        m_resolution = QSize(1920, 1080);
        return;
    }
    m_refreshRate   = screen->refreshRate();
    m_resolution    = screen->size();
    m_physicalSize  = QSize(static_cast<int>(screen->physicalSize().width()),
                            static_cast<int>(screen->physicalSize().height()));
}

/// \brief Return true if the MythTV windows should span all screens.
bool MythDisplay::SpanAllScreens(void)
{
    return gCoreContext->GetSetting("XineramaScreen", nullptr) == "-1";
}

QString MythDisplay::GetExtraScreenInfo(QScreen *qScreen)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    QString mfg = qScreen->manufacturer();
    if (mfg.isEmpty())
        mfg = "Unknown";
    QString model = qScreen->model();
    if (model.isEmpty())
        model = "Unknown";
    return QString("(Make: %1 Model: %2)").arg(mfg).arg(model);
#else
    Q_UNUSED(qScreen);
    return QString();
#endif
}

void MythDisplay::DebugScreen(QScreen *qScreen, const QString &Message)
{
    if (!qScreen)
        return;

    QRect geom = qScreen->geometry();
    QString extra = GetExtraScreenInfo(qScreen);

    LOG(VB_GENERAL, LOG_INFO, LOC + QString("%1 screen '%2' %3")
        .arg(Message).arg(qScreen->name()).arg(extra));

    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Geometry %1x%2+%3+%4 Size %5mmx%6mm")
        .arg(geom.width()).arg(geom.height()).arg(geom.left()).arg(geom.top())
        .arg(qScreen->physicalSize().width()).arg(qScreen->physicalSize().height()));

    if (qScreen->virtualGeometry() != geom)
    {
        geom = qScreen->virtualGeometry();
        LOG(VB_GENERAL, LOG_INFO, LOC + QString("Total virtual geometry: %1x%2+%3+%4")
            .arg(geom.width()).arg(geom.height()).arg(geom.left()).arg(geom.top()));
    }
}

void MythDisplay::Initialise(void)
{
    m_videoModes.clear();
    m_overrideVideoModes.clear();
    UpdateCurrentMode();

    // Set the desktop mode - which is the mode at startup. We must always return
    // the screen to this mode.
    if (!m_initialised)
    {
        // Only ever set this once or after a screen change
        m_initialised = true;
        m_desktopMode = MythDisplayMode(m_resolution, m_physicalSize, -1.0, m_refreshRate);
        LOG(VB_GENERAL, LOG_NOTICE, LOC + QString("Desktop video mode: %1x%2 %3Hz")
            .arg(m_resolution.width()).arg(m_resolution.height()).arg(m_refreshRate, 0, 'f', 3));
        if (m_edid.Valid())
        {
            if (m_edid.IsSRGB())
                LOG(VB_GENERAL, LOG_NOTICE, LOC + "Display is using sRGB colourspace");
            else
                LOG(VB_GENERAL, LOG_NOTICE, LOC + "Display has custom colourspace");
        }
    }

    // Set the gui mode from database settings
    int pixelwidth     = m_resolution.width();
    int pixelheight    = m_resolution.height();
    int mmwidth        = m_physicalSize.width();
    int mmheight       = m_physicalSize.height();
    double refreshrate = m_refreshRate;
    double aspectratio = 0.0;
    GetMythDB()->GetResolutionSetting("GuiVidMode", pixelwidth, pixelheight, aspectratio, refreshrate);
    GetMythDB()->GetResolutionSetting("DisplaySize", mmwidth, mmheight);
    m_guiMode = MythDisplayMode(pixelwidth, pixelheight, mmwidth, mmheight, -1.0, refreshrate);

    // Set default video mode
    pixelwidth = pixelheight = 0;
    GetMythDB()->GetResolutionSetting("TVVidMode", pixelwidth, pixelheight, aspectratio, refreshrate);
    m_videoMode = MythDisplayMode(pixelwidth, pixelheight, mmwidth, mmheight, aspectratio, refreshrate);

    // Initialise video override modes
    for (int i = 0; true; ++i)
    {
        int iw = 0;
        int ih = 0;
        int ow = 0;
        int oh = 0;
        double iaspect = 0.0;
        double oaspect = 0.0;
        double irate = 0.0;
        double orate = 0.0;

        GetMythDB()->GetResolutionSetting("VidMode", iw, ih, iaspect, irate, i);
        GetMythDB()->GetResolutionSetting("TVVidMode", ow, oh, oaspect, orate, i);

        if (!(iw || ih || !qFuzzyIsNull(irate)) || !(ih && ow && oh))
            break;

        uint64_t key = MythDisplayMode::CalcKey(QSize(iw, ih), irate);
        MythDisplayMode scr(QSize(ow, oh), QSize(mmwidth, mmheight), oaspect, orate);
        m_overrideVideoModes[key] = scr;
    }
}

/*! \brief Check whether the next mode is larger in size than the current mode.
 *
 * This is used to allow the caller to force an update of the main window to ensure
 * the window is fully resized and is in no way clipped. Not an issue if the next
 * mode is smaller.
*/
bool MythDisplay::NextModeIsLarger(QSize Size)
{
    return Size.width() > m_resolution.width() || Size.height() > m_resolution.height();
}

/*! \brief Return the screen to the original desktop video mode
 *
 * \note It is assume the app is exiting once this is called.
*/
void MythDisplay::SwitchToDesktop()
{
    MythDisplayMode current(m_resolution, m_physicalSize, -1.0, m_refreshRate);
    if (current == m_desktopMode)
        return;
    SwitchToVideoMode(m_desktopMode.Resolution(), m_desktopMode.RefreshRate());
}

/** \brief Switches to the resolution and refresh rate defined in the
 * database for the specified video resolution and frame rate.
*/
bool MythDisplay::SwitchToVideo(QSize Size, double Rate)
{
    MythDisplayMode next = m_videoMode;
    MythDisplayMode current(m_resolution, m_physicalSize, -1.0, m_refreshRate);
    double targetrate = 0.0;
    double aspectoverride = 0.0;

    // try to find video override mode
    uint64_t key = MythDisplayMode::FindBestScreen(m_overrideVideoModes,
                   Size.width(), Size.height(), Rate);

    if (key != 0)
    {
        next = m_overrideVideoModes[key];
        if (next.AspectRatio() > 0.0)
            aspectoverride = next.AspectRatio();
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Found custom screen override %1x%2 Aspect %3")
            .arg(next.Width()).arg(next.Height()).arg(aspectoverride));
    }

    // If requested refresh rate is 0, attempt to match video fps
    if (qFuzzyIsNull(next.RefreshRate()))
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Trying to match best refresh rate %1Hz")
            .arg(Rate, 0, 'f', 3));
        next.SetRefreshRate(Rate);
    }

    // need to change video mode?
    MythDisplayMode::FindBestMatch(GetVideoModes(), next, targetrate);
    if ((next == current) && (MythDisplayMode::CompareRates(current.RefreshRate(), targetrate)))
    {
        LOG(VB_GENERAL, LOG_INFO, LOC + QString("Using current mode %1x%2@%3Hz")
            .arg(m_resolution.width()).arg(m_resolution.height()).arg(m_refreshRate));
        return true;
    }

    LOG(VB_GENERAL, LOG_ERR, LOC + QString("Trying mode %1x%2@%3Hz")
        .arg(next.Width()).arg(next.Height()).arg(next.RefreshRate(), 0, 'f', 3));

    if (!SwitchToVideoMode(next.Resolution(), targetrate))
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + QString("Failed to change mode to %1x%2@%3Hz")
            .arg(next.Width()).arg(next.Height()).arg(next.RefreshRate(), 0, 'f', 3));
        return false;
    }

    if (next.Resolution() != m_resolution)
        WaitForScreenChange();

    // N.B. We used a computed aspect ratio unless overridden
    m_aspectRatio = aspectoverride > 0.0 ? aspectoverride : 0.0;
    UpdateCurrentMode();
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Switched to %1x%2@%3Hz for video %4x%5")
        .arg(m_resolution.width()).arg(m_resolution.height())
        .arg(m_refreshRate, 0, 'f', 3).arg(Size.width()).arg(Size.height()));
    PauseForModeSwitch();
    return true;
}

/** \brief Switches to the GUI resolution.
 */
bool MythDisplay::SwitchToGUI(bool Wait)
{
    // If the current resolution is the same as the GUI resolution then do nothing
    // as refresh rate should not be critical for the GUI.
    if (m_resolution == m_guiMode.Resolution())
    {
        LOG(VB_GENERAL, LOG_INFO, LOC + QString("Using %1x%2@%3Hz for GUI")
            .arg(m_resolution.width()).arg(m_resolution.height()).arg(m_refreshRate));
        return true;
    }

    if (!SwitchToVideoMode(m_guiMode.Resolution(), m_guiMode.RefreshRate()))
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + QString("Failed to change mode to %1x%2@%3Hz")
            .arg(m_guiMode.Width()).arg(m_guiMode.Height()).arg(m_guiMode.RefreshRate(), 0, 'f', 3));
        return false;
    }

    if (Wait && (m_resolution != m_guiMode.Resolution()))
        WaitForScreenChange();

    UpdateCurrentMode();
    m_aspectRatio = 0.0;
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Switched to %1x%2@%3Hz")
        .arg(m_resolution.width()).arg(m_resolution.height()).arg(m_refreshRate, 0, 'f', 3));
    return true;
}

double MythDisplay::GetRefreshRate(void)
{
    return m_refreshRate;
}

int MythDisplay::GetRefreshInterval(int Fallback)
{
    if (m_refreshRate > 20.0 && m_refreshRate < 200.0)
        return static_cast<int>(lround(1000000.0 / m_refreshRate));
    return Fallback;
}

std::vector<double> MythDisplay::GetRefreshRates(QSize Size)
{
    auto targetrate = static_cast<double>(NAN);
    const MythDisplayMode mode(Size, QSize(0, 0), -1.0, 0.0);
    const vector<MythDisplayMode>& modes = GetVideoModes();
    int match = MythDisplayMode::FindBestMatch(modes, mode, targetrate);
    if (match < 0)
        return std::vector<double>();
    return modes[static_cast<size_t>(match)].RefreshRates();
}

bool MythDisplay::SwitchToVideoMode(QSize, double)
{
    return false;
}

const vector<MythDisplayMode> &MythDisplay::GetVideoModes(void)
{
    return m_videoModes;
}

/** \brief Returns current screen aspect ratio.
 *
 * If there is an aspect overide in the database that aspect ratio is returned
 * instead of the actual screen aspect ratio.
*/
double MythDisplay::GetAspectRatio(void)
{
    if (m_aspectRatio > 0.0)
        return m_aspectRatio;

    if (m_physicalSize.width() > 0 && m_physicalSize.height() > 0)
        return static_cast<double>(m_physicalSize.width()) / m_physicalSize.height();

    return 16.0 / 9.0;
}

MythEDID& MythDisplay::GetEDID(void)
{
    return m_edid;
}

/*! \brief Estimate the overall display aspect ratio for multi screen setups.
 *
 * \note This will only work where screens are configured either as a grid (e.g. 2x2)
 * or as a single 'row' or 'column' of screens. Likewise it will fail if the aspect
 * ratios of the displays are not similar and may not work if the display
 * resolutions are significantly different (in a grid type setup).
 *
 * \note Untested with a grid layout - anyone have a card with 4 outputs?
*/
double MythDisplay::EstimateVirtualAspectRatio(void)
{
    auto sortscreens = [](const QScreen* First, const QScreen* Second)
    {
        if (First->geometry().left() < Second->geometry().left())
            return true;
        if (First->geometry().top() < Second->geometry().top())
            return true;
        return false;
    };

    // default
    auto result = GetAspectRatio();

    QList<QScreen*> screens;
    if (m_screen)
        screens = m_screen->virtualSiblings();
    if (screens.size() < 2)
        return result;

    // N.B. This sorting may not be needed
    std::sort(screens.begin(), screens.end(), sortscreens);
    QList<double> aspectratios;
    int lasttop = 0;
    int lastleft = 0;
    int rows = 1;
    int columns = 1;
    for (auto it = screens.constBegin() ; it != screens.constEnd(); ++it)
    {
        QRect geom = (*it)->geometry();
        LOG(VB_PLAYBACK, LOG_DEBUG, LOC + QString("%1x%2+%3+%4 %5")
            .arg(geom.width()).arg(geom.height()).arg(geom.left()).arg(geom.top())
            .arg((*it)->physicalSize().width() / (*it)->physicalSize().height()));
        if (lastleft < geom.left())
        {
            columns++;
            lastleft = geom.left();
        }
        if (lasttop < geom.top())
        {
            rows++;
            lasttop = geom.top();
            lastleft = 0;
        }
        aspectratios << (*it)->physicalSize().width() / (*it)->physicalSize().height();
    }

    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Screen layout: %1x%2").arg(rows).arg(columns));
    if (rows == columns)
    {
        LOG(VB_GENERAL, LOG_DEBUG, LOC + "Grid layout");
    }
    else if (rows == 1 && columns > 1)
    {
        LOG(VB_GENERAL, LOG_DEBUG, LOC + "Horizontal layout");
    }
    else if (columns == 1 && rows > 1)
    {
        LOG(VB_GENERAL, LOG_DEBUG, LOC + "Vertical layout");
    }
    else
    {
        LOG(VB_GENERAL, LOG_INFO,
            LOC + QString("Unsupported layout - defaulting to %1")
            .arg(result));
        return result;
    }

    // validate aspect ratios - with a little fuzzyness
    double aspectratio = 0.0;
    double average = 0.0;
    int count = 1;
    for (auto it2 = aspectratios.constBegin() ; it2 != aspectratios.constEnd(); ++it2, ++count)
    {
        aspectratio += *it2;
        average = aspectratio / count;
        if (qAbs(*it2 - average) > 0.1)
        {
            LOG(VB_GENERAL, LOG_INFO, LOC +
                QString("Inconsistent aspect ratios - defaulting to %1")
                .arg(result));
            return result;
        }
    }

    aspectratio = (average * columns) / rows;
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Estimated aspect ratio: %1")
        .arg(aspectratio));
    return aspectratio;
}

QSize MythDisplay::GetResolution(void)
{
    return m_resolution;
}

QSize MythDisplay::GetPhysicalSize(void)
{
    return m_physicalSize;
}

void MythDisplay::WaitForScreenChange(void)
{
    LOG(VB_GENERAL, LOG_INFO, LOC + "Waiting for resolution change");
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, [](){ LOG(VB_GENERAL, LOG_WARNING, LOC + "Timed out wating for screen change"); });
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(m_screen, &QScreen::geometryChanged, &loop, &QEventLoop::quit);
    // 500ms maximum wait
    timer.start(500);
    loop.exec();
}

void MythDisplay::PauseForModeSwitch(void)
{
    int pauselengthinms = gCoreContext->GetNumSetting("VideoModeChangePauseMS", 0);
    if (pauselengthinms)
    {
        LOG(VB_GENERAL, LOG_INFO, LOC +
            QString("Pausing %1ms for video mode switch").arg(pauselengthinms));
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        // 500ms maximum wait
        timer.start(pauselengthinms);
        loop.exec();
    }
}

void MythDisplay::DebugModes(void) const
{
    // This is intentionally formatted to match the output of xrandr for comparison
    if (VERBOSE_LEVEL_CHECK(VB_PLAYBACK, LOG_INFO))
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + "Available modes:");
        auto it = m_videoModes.crbegin();
        for ( ; it != m_videoModes.crend(); ++it)
        {
            auto rates = (*it).RefreshRates();
            QStringList rateslist;
            auto it2 = rates.crbegin();
            for ( ; it2 != rates.crend(); ++it2)
                rateslist.append(QString("%1").arg(*it2, 2, 'f', 2, '0'));
            LOG(VB_PLAYBACK, LOG_INFO, QString("%1x%2\t%3")
                .arg((*it).Width()).arg((*it).Height()).arg(rateslist.join("\t")));
        }
    }
}

