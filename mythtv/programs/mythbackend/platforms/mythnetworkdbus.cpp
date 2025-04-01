/*
 *  Class MythNetworkDBus
 *
 *  Copyright (c) David Hampton 2021,2025
 *
 *  Licensed under the GPL v2 or later, see COPYING for details
 */

// D-Bus API Reference
//
// https://developer-old.gnome.org/NetworkManager/stable/gdbus-org.freedesktop.NetworkManager.Device.html
// https://people.freedesktop.org/~lkundrak/nm-docs/spec.html


#include <algorithm>
#include <iostream>

#include <QDebug>
#include <QDBusArgument>

#include "libmythbase/mythcorecontext.h"
#include "libmythbase/mythlogging.h"

#include "mythnetworkdbus.h"
#include "mythbackend_main_helpers.h"

static const QString nmService    { QStringLiteral(u"org.freedesktop.NetworkManager") };
static const QString nmPath       { QStringLiteral(u"/org/freedesktop/NetworkManager") };
static const QString nmInterface  { QStringLiteral(u"org.freedesktop.NetworkManager") };
static const QString nmDInterface { QStringLiteral(u"org.freedesktop.NetworkManager.Device") };

#define LOC QString("NetworkDBus: ")

//////////////////////////////////////////////////

QMap<quint32,QString> MythNetworkDevice::NMDeviceStateName = {
    { NM_DEVICE_STATE_UNKNOWN, "Unknown"},
    { NM_DEVICE_STATE_UNMANAGED, "Unmanaged"},
    { NM_DEVICE_STATE_UNAVAILABLE, "Unavailable"},
    { NM_DEVICE_STATE_DISCONNECTED, "Disconnected"},
    { NM_DEVICE_STATE_PREPARE, "Prepare"},
    { NM_DEVICE_STATE_CONFIG, "Config"},
    { NM_DEVICE_STATE_NEED_AUTH, "Need_auth"},
    { NM_DEVICE_STATE_IP_CONFIG, "IP_config"},
    { NM_DEVICE_STATE_IP_CHECK, "IP_check"},
    { NM_DEVICE_STATE_SECONDARIES, "Secondaries"},
    { NM_DEVICE_STATE_ACTIVATED, "Activated"},
    { NM_DEVICE_STATE_DEACTIVATING, "Deactivating"},
    { NM_DEVICE_STATE_FAILED, "Failed"},
};

QMap<quint32,QString> MythNetworkDevice::NMDeviceStateReason = {
    {  0, "none given"},
    {  1, "unknown"},
    {  2, "now managed"},
    {  3, "now unmanaged"},
    {  4, "IP config failed"},
    {  5, "disconnected"},
    { 36, "removed"},
    { 37, "sleeping"},
    { 38, "connection removed"},
    { 39, "user requested"},
    { 40, "carrier/link changed"},
    { 44, "bluetooth failed"},
    { 54, "secondary failed"},
    { 60, "new activation"},
    { 61, "parent changed"},
    { 62, "parent management changed"},
};



static uint32_t pathToIndex(const QString& path)
{
    int slash = path.lastIndexOf('/');
    return path.mid(slash+1).toInt();
}


MythNetworkDevice::MythNetworkDevice(const QString& path, QObject *parent)
    : QObject(parent),
      m_path(path),
      m_index(pathToIndex(path))
{
    auto sysBus = QDBusConnection::systemBus();
    QDBusInterface device(nmService, m_path, nmDInterface, sysBus);
    m_name = device.property("Interface").toString();
    m_state = device.property("State").toInt();

    QString stateStr = NMDeviceStateName.value(m_state, QString::number(m_state));
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Interface %1:%2, state %3)")
        .arg(m_index).arg(m_name, stateStr));

    if (!sysBus.connect(QString(), m_path, nmDInterface, "StateChanged",
                        this, SLOT(StateChanged(quint32,quint32,quint32))))
        LOG(VB_GENERAL, LOG_WARNING, LOC +
            QString("Listening for network interface state changes failed."));

    m_valid = true;
}

QString MythNetworkDevice::getInfo(void)
{
    QString stateStr = NMDeviceStateName.value(m_state, QString::number(m_state));
    return QString("Interface %%2, state %3)")
        .arg(m_name, stateStr);
}

/// Process a notification that a network device has changed state. If
/// a managed device has moved to the "activated" state because the
/// network has come up, then MythTV should re-check to see if any
/// encoders are now reachable. Also do this if an interface is no
/// longer being managed by Network Manager and is now considered
/// "unmanaged".
///
/// @param newState[in] The new state of the interface device.
/// @param oldState[in] The old state of the interface device.
/// @param reason[in] The reason for the change of state.
void MythNetworkDevice::StateChanged(quint32 newState, quint32 oldState, quint32 reason)
{
    QString reasonStr = NMDeviceStateReason.contains(reason)
        ? NMDeviceStateReason[reason]
        : QString::number(reason);
    // Eventually make this LOG_DEBUG
    LOG(VB_GENERAL, LOG_INFO,
        QString(LOC + "Network %1/%2 changed state (%3 -> %4), reason %5.")
        .arg(m_index)
        .arg(m_name,
             NMDeviceStateName[oldState],
             NMDeviceStateName[newState],
             reasonStr));

    if ((newState != NM_DEVICE_STATE_UNMANAGED) &&
        (newState != NM_DEVICE_STATE_ACTIVATED))
        return;

    // DO SOMETHING BASED ON THE STATE CHANGE
    bool ismaster = gCoreContext->IsMasterHost();
    createTVRecorders(ismaster, true);
}

//////////////////////////////////////////////////

MythNetworkDevice *MythNetworkDBus::findByIndex (uint32_t index)
{
    auto matchIndex = [index](QObject *c)
        {
            auto * dev = qobject_cast<MythNetworkDevice*>(c);
            return dev ? index == dev->getIndex() : false;
        };
    auto iter = std::find_if(children().cbegin(), children().cend(), matchIndex);
    if (iter == children().cend())
        return nullptr;
    return qobject_cast<MythNetworkDevice*>(*iter);
}

MythNetworkDevice *MythNetworkDBus::findByPath (const QString& path)
{
    return(findByIndex(pathToIndex(path)));
}

void MythNetworkDBus::printChildList (void)
{
    LOG(VB_GENERAL, LOG_INFO, LOC + "Children:");
    for (auto * d : std::as_const(children()))
    {
        auto *dev = qobject_cast<MythNetworkDevice*>(d);
        LOG(VB_GENERAL, LOG_INFO, LOC + dev->getInfo());
    }
}

/// Create a new (singleton) MythNetworkDBus object. This object will
/// obtain a list of all network devices, track added/removed devices,
/// and track state changes on these devices.
///
/// @param parent[in] The parent object (should be the app).
MythNetworkDBus::MythNetworkDBus(QObject *parent)
    : QObject(parent)
{
    auto sysBus = QDBusConnection::systemBus();
    if (!sysBus.isConnected())
        return;

    m_networkInterface = new QDBusInterface(nmService, nmPath, nmInterface, sysBus);
    if (!m_networkInterface)
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + "No Network interface. Unable to monitor network interface state changes.");
        return;
    }

    // Get all current interfaces
    auto reply = m_networkInterface->call(QLatin1String("GetDevices"));
    auto arg = reply.arguments().at(0).value<QDBusArgument>();
    if(arg.currentType() != QDBusArgument::ArrayType)
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + "Error parsing the device list");
        return;
    }
    auto pathsLst = qdbus_cast<QList<QDBusObjectPath>>(arg);
    for(const auto& p : std::as_const(pathsLst))
        new MythNetworkDevice(p.path(), qobject_cast<QObject*>(this));

    // Set up to track added/removed interfaces.
    if (!sysBus.connect(QString(), nmPath, nmInterface, "DeviceAdded",
                        this, SLOT(DeviceAdded(QDBusObjectPath))) ||
        !sysBus.connect(QString(), nmPath, nmInterface, "DeviceRemoved",
                        this, SLOT(DeviceRemoved(QDBusObjectPath))))
        LOG(VB_GENERAL, LOG_WARNING, LOC +
            QString("Listening for added/removed network interfaces failed."));
}

/// Process a notification that a network device has added to the
/// system. Add it to the internal list of devices (if not already
/// present) and start tracking state changes.
///
/// @param dPath[in] The dbus name for the added interface.
void MythNetworkDBus::DeviceAdded(const QDBusObjectPath& dPath)
{
    QString path = dPath.path();
    if (nullptr != findByPath(path))
    {
        LOG(VB_GENERAL, LOG_ERR, LOC +
            QString("Device %1 already exists.").arg(path));
        return;
    }
    new MythNetworkDevice(path, qobject_cast<QObject*>(this));
}

/// Process a notification that a network device has removed from the
/// system. Remove it from the internal list of devices.
///
/// @param dPath[in] The dbus name for the added interface.
void MythNetworkDBus::DeviceRemoved(const QDBusObjectPath& dPath)
{
    delete findByPath(dPath.path());
}
