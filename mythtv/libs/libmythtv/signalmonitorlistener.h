// -*- Mode: c++ -*-
// Copyright (c) 2008, Daniel Thor Kristjansson

#ifndef SIGNALMONITORLISTENER_H
#define SIGNALMONITORLISTENER_H

#include "mythtvexp.h"
#include "signalmonitorvalue.h"

enum SignalMonitorMessageType : std::uint8_t {
    kAllGood,
    kStatusChannelTuned,
    kStatusSignalLock,
    kStatusSignalStrength,
    kStatusSignalToNoise,
    kStatusBitErrorRate,
    kStatusUncorrectedBlocks,
    kStatusRotorPosition,
};

class MTV_PUBLIC SignalMonitorListener
{
  protected:
    virtual ~SignalMonitorListener() = default;

  public:
    /** \brief Signal to be sent when you have a lock on all values.
     *
     *   Note: Signals are only sent once the monitoring thread
     *         has been started.
     */
    virtual void AllGood(void) = 0;

    /** \brief Signal to be sent with change change status.
     *
     *   Note: Signals are only sent once the monitoring thread
     *         has been started.
     */
    virtual void StatusChannelTuned(const SignalMonitorValue&) = 0;

    /** \brief Signal to be sent as true when it is safe to begin
     *   or continue recording, and false if it may not be safe.
     *
     *   Note: Signals are only sent once the monitoring thread
     *         has been started.
     */

    virtual void StatusSignalLock(const SignalMonitorValue&) = 0;
    /** \brief Signal to be sent with an actual signal value.
     *
     *   Note: Signals are only sent once the monitoring thread
     *         has been started.
     */
    virtual void StatusSignalStrength(const SignalMonitorValue&) = 0;
};

class MTV_PUBLIC DVBSignalMonitorListener : public SignalMonitorListener
{
  protected:
    ~DVBSignalMonitorListener() override = default;

  public:
    virtual void StatusSignalToNoise(    const SignalMonitorValue&) = 0;
    virtual void StatusBitErrorRate(     const SignalMonitorValue&) = 0;
    virtual void StatusUncorrectedBlocks(const SignalMonitorValue&) = 0;
    virtual void StatusRotorPosition(    const SignalMonitorValue&) = 0;
};


#endif // SIGNALMONITORLISTENER_H
