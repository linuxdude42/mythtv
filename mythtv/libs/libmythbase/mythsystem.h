/* -*- Mode: c++ -*-
 *  Class MythSystem
 *
 *  Copyright (C) Daniel Kristjansson 2013
 *  Copyright (C) Gavin Hurlbut 2012
 *  Copyright (C) Issac Richards 2008
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef MYTHSYSTEM_H_
#define MYTHSYSTEM_H_

// Qt headers
#include <QIODevice>
#include <QString>

// MythTV headers
#include "mythbaseexp.h"
#include "mythchrono.h"

enum MythSystemFlag {
    kMSNone               = 0x00000000,
    kMSDontBlockInputDevs = 0x00000001, ///< avoid blocking LIRC & Joystick Menu
    kMSDontDisableDrawing = 0x00000002, ///< avoid disabling UI drawing
    kMSRunBackground      = 0x00000004, ///< run child in the background
    kMSProcessEvents      = 0x00000008, ///< process events while waiting
    kMSStdIn              = 0x00000020, ///< allow access to stdin
    kMSStdOut             = 0x00000040, ///< allow access to stdout
    kMSStdErr             = 0x00000080, ///< allow access to stderr
    kMSRunShell           = 0x00000200, ///< run process through shell
    kMSAnonLog            = 0x00000800, ///< anonymize the logs
    kMSAutoCleanup        = 0x00004000, ///< automatically delete if
                                        ///  backgrounded
    kMSLowExitVal         = 0x00008000, ///< allow exit values 0-127 only
    // ^ FIXME Eliminate? appears to be a hack for some "ubuntu", but
    //                    doesn't appear to be needed with ubuntu 12.04
    kMSDisableUDPListener = 0x00010000, ///< disable MythMessage UDP listener
                                        ///  for the duration of application.
    kMSPropagateLogs      = 0x00020000, ///< add arguments for MythTV log
                                        ///  propagation
};

enum MythSignal : std::uint8_t {
    kSignalNone,
    kSignalUnknown,
    kSignalHangup,
    kSignalInterrupt,
    kSignalContinue,
    kSignalQuit,
    kSignalSegfault,
    kSignalKill,
    kSignalUser1,
    kSignalUser2,
    kSignalTerm,
    kSignalStop,
};

/** \brief class for managing sub-processes.
 *
 *  This is a hopefully simple interface for managing sub-processes.
 *
 *  The general usage is:
 *  {
 *      QScopedPointer sys(MythSystem::Create("touch /tmp/i.am.legend"));
 *  }
 */
class MBASE_PUBLIC MythSystem
{
  public:
    /// Priorities that can be used for cpu and disk usage of child process
    enum Priority : std::uint8_t {
        kIdlePriority   = 0, ///< run only when no one else wants to
        kLowestPriority,
        kLowPriority,
        kNormalPriority,   ///< run as a normal program
        kHighPriority,
        kHighestPriority,
        kTimeCriticalPriority,
        kInheritPriority, ///< Use parent priority
    };

    static MythSystem *Create(
        const QStringList &args,
        uint flags = kMSNone,
        const QString& startPath = QString(),
        Priority cpuPriority = kInheritPriority,
        Priority diskPriority = kInheritPriority);

    static MythSystem *Create(
        const QString& args,
        uint flags = kMSNone,
        const QString& startPath = QString(),
        Priority cpuPriority = kInheritPriority,
        Priority diskPriority = kInheritPriority);

    virtual ~MythSystem(void) = default;

    // Deleted functions should be public.
    MythSystem(const MythSystem&) = delete; // no-implementation
    MythSystem& operator= (const MythSystem&) = delete; // no-implementation

    /// Returns the flags passed to the constructor
    virtual uint GetFlags(void) const = 0;

    /// Returns the starting path of the program
    virtual QString GetStartingPath(void) const = 0;

    /// Return the CPU Priority of the program
    virtual Priority GetCPUPriority(void) const = 0;

    /// Return the Disk Priority of the program
    virtual Priority GetDiskPriority(void) const = 0;

    /** Blocks until child process is collected or timeout reached.
     *  If the timeout is not provided or a timeout of 0 is provided
     *  this will block until the sub-program exits.
     *  \return true if program has exited and has been collected.
     */
    virtual bool Wait(std::chrono::milliseconds timeout = 0ms) = 0;

    /// Returns the standard input stream for the program
    /// if the kMSStdIn flag was passed to the constructor.
    /// Note: The stream this returns is already open.
    virtual QIODevice *GetStandardInputStream(void) = 0;

    /// Returns the standard output stream for the program
    /// if the kMSStdOut flag was passed to the constructor.
    /// Note: The stream this returns is already open.
    virtual QIODevice *GetStandardOutputStream(void) = 0;

    /// Returns the standard error stream for the program
    /// if the kMSStdErr flag was passed to the constructor.
    /// Note: The stream this returns is already open.
    virtual QIODevice *GetStandardErrorStream(void) = 0;

    /// Sends the selected signal to the program
    virtual void Signal(MythSignal) = 0;

    /** \brief returns the exit code, if any, that the program returned.
     *
     *  Returns -1 if the program exited without exit code.
     *  Returns -2 if the program has not yet been collected.
     *  Returns an exit code 0..255 if the program exited with exit code.
     */
    virtual int GetExitCode(void) const = 0;

  protected:
    MythSystem() = default;
};

#endif // MYTHSYSTEM_H_

/* vim:ts=4:sw=4:ai:et:si:sts=4 */
