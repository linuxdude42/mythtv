//myth
#include "libmythbase/mythcorecontext.h"
#include "libmythbase/mythdirs.h"

// mythwelcome
#include "welcomesettings.h"

///////////////////////////////////////////////////////////////////
//  daily wakeup/shutdown settings
///////////////////////////////////////////////////////////////////

#define TR MythWelcomeSettings::tr

namespace {
StandardSetting *DailyWakeupStart(const QString& n)
{
    auto *gc = new GlobalTimeBoxSetting("DailyWakeupStartPeriod" + n, "00:00");
    gc->setLabel(TR("Period %1 start time").arg(n));
    gc->setHelpText(TR("Period %1 start time. "
                       "Defines a period the master backend should be awake. "
                       "Set both Start & End times to 00:00 to disable.").arg(n));
    return gc;
}

StandardSetting *DailyWakeupEnd(const QString& n)
{
    auto *gc = new GlobalTimeBoxSetting("DailyWakeupEndPeriod" + n, "00:00");
    gc->setLabel(TR("Period %1 end time").arg(n));
    gc->setHelpText(TR("Period %1 end time. "
                       "Defines a period the master backend should be awake. "
                       "Set both Start & End times to 00:00 to disable.").arg(n));
    return gc;
}

StandardSetting *DailyWakeup(const QString& n)
{
    auto *gc = new GroupSetting();

    gc->setLabel(TR("Daily Wakeup/ShutDown Period %1").arg(n));
    gc->addChild(DailyWakeupStart(n));
    gc->addChild(DailyWakeupEnd(n));
    return gc;
}

StandardSetting *AutoStartFrontend()
{
    auto *gc = new HostCheckBoxSetting("AutoStartFrontend");
    gc->setLabel(TR("Automatically Start mythfrontend"));
    gc->setValue(true);
    gc->setHelpText(TR("Mythwelcome will automatically "
                       "start mythfrontend if it is determined that it was "
                       "not started to record a program."));
    return gc;
}

StandardSetting *ShutdownWithBE()
{
    auto *gc = new HostCheckBoxSetting("ShutdownWithMasterBE");
    gc->setLabel(TR("Shutdown with Master Backend"));
    gc->setValue(false);
    gc->setHelpText(TR("Mythwelcome will automatically "
                       "shutdown this computer when the master backend shuts "
                       "down. Should only be set on frontend only machines"));
    return gc;
}
} // end anonymous namespace

MythWelcomeSettings::MythWelcomeSettings()
{
    setLabel(tr("MythWelcome Settings"));

    addChild(DailyWakeup("1"));
    addChild(DailyWakeup("2"));

    addChild(AutoStartFrontend());

    // this setting only makes sense on frontend only machines
    if (gCoreContext->IsFrontendOnly())
        addChild(ShutdownWithBE());
}

///////////////////////////////////////////////////////////////////
//  mythshutdown script settings
///////////////////////////////////////////////////////////////////

#undef TR
#define TR MythShutdownSettings::tr

namespace {
StandardSetting *MythShutdownNvramCmd()
{
    auto *gc = new HostTextEditSetting("MythShutdownNvramCmd");
    gc->setLabel(TR("Command to Set Wakeup Time"));
    gc->setValue("/usr/bin/nvram-wakeup --settime $time");
    gc->setHelpText(TR("Command to set the wakeup time "
                       "in the BIOS. See the README file for more examples."));
    return gc;
}

StandardSetting *WakeupTimeFormat()
{
    auto *gc = new HostComboBoxSetting("MythShutdownWakeupTimeFmt", true);
    gc->setLabel(TR("Wakeup time format"));
    gc->addSelection("time_t");
    gc->addSelection("yyyy-MM-dd hh:mm:ss");
    gc->setHelpText(TR("The format of the time string "
                       "passed to the \'Set Wakeup Time Command\' as $time. "
                       "See QT::QDateTime.toString() for details. Set to "
                       "'time_t' for seconds since epoch (use time_t for "
                       "nvram_wakeup)."));
    return gc;
}

StandardSetting *MythShutdownNvramRestartCmd()
{
    auto *gc = new HostTextEditSetting("MythShutdownNvramRestartCmd");
    gc->setLabel(TR("nvram-wakeup Restart Command"));
    gc->setValue("/sbin/grub-set-default 1");
    gc->setHelpText(TR("Command to run if your bios "
                       "requires you to reboot to allow nvram-wakeup settings "
                       "to take effect. Leave blank if your bios does not "
                       "require a reboot. See the README file for more "
                       "examples."));
    return gc;
}

StandardSetting *MythShutdownReboot()
{
    auto *gc = new HostTextEditSetting("MythShutdownReboot");
    gc->setLabel(TR("Command to reboot"));
    gc->setValue("/sbin/reboot");
    gc->setHelpText(TR("Command to reboot computer."));
    return gc;
}

StandardSetting *MythShutdownPowerOff()
{
    auto *gc = new HostTextEditSetting("MythShutdownPowerOff");
    gc->setLabel(TR("Command to shutdown"));
    gc->setValue("/sbin/poweroff");
    gc->setHelpText(TR("Command to shutdown computer."));
    return gc;
}

StandardSetting *MythShutdownStartFECmd()
{
    auto *gc = new HostTextEditSetting("MythWelcomeStartFECmd");
    gc->setLabel(TR("Command to run to start the Frontend"));
    gc->setValue(GetAppBinDir() + "mythfrontend");
    gc->setHelpText(TR("Command to start mythfrontend."));
    return gc;
}

StandardSetting *MythShutdownXTermCmd()
{
    auto *gc = new HostTextEditSetting("MythShutdownXTermCmd");
    gc->setLabel(TR("Command to run Xterm"));
    gc->setValue("xterm");
    gc->setHelpText(TR("Command to start an Xterm. Can "
                       "be disabled by leaving this "
                       "setting blank."));
    return gc;
}
} // end anonymous namespace

MythShutdownSettings::MythShutdownSettings()
{
    setLabel(tr("MythShutdown/MythWelcome Settings"));
    addChild(MythShutdownNvramCmd());
    addChild(WakeupTimeFormat());
    addChild(MythShutdownNvramRestartCmd());
    addChild(MythShutdownReboot());
    addChild(MythShutdownPowerOff());
    addChild(MythShutdownXTermCmd());
    addChild(MythShutdownStartFECmd());
}
