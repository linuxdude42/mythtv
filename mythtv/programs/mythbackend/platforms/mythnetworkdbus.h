/*
 *  Class MythNetworkDBus
 *
 *  Copyright (c) David Hampton 2021,2025
 *
 *  Licensed under the GPL v2 or later, see COPYING for details
 */

#ifndef MYTHNETWORKDBUS_H
#define MYTHNETWORKDBUS_H

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QObject>
#include <QString>

class MythNetworkDevice : public QObject
{
    Q_OBJECT;

  public:
    MythNetworkDevice(const QString& path, QObject *parent = nullptr);
    uint32_t getIndex() const { return m_index; };
    QString getInfo();

  private:
    enum NMDeviceState : std::uint8_t {
        NM_DEVICE_STATE_UNKNOWN = 0,
        NM_DEVICE_STATE_UNMANAGED = 10,
        NM_DEVICE_STATE_UNAVAILABLE = 20,
        NM_DEVICE_STATE_DISCONNECTED = 30,
        NM_DEVICE_STATE_PREPARE = 40,
        NM_DEVICE_STATE_CONFIG = 50,
        NM_DEVICE_STATE_NEED_AUTH = 60,
        NM_DEVICE_STATE_IP_CONFIG = 70,
        NM_DEVICE_STATE_IP_CHECK = 80,
        NM_DEVICE_STATE_SECONDARIES = 90,
        NM_DEVICE_STATE_ACTIVATED = 100,
        NM_DEVICE_STATE_DEACTIVATING = 110,
        NM_DEVICE_STATE_FAILED = 120,
    };
    static QMap<quint32,QString> NMDeviceStateName;
    static QMap<quint32,QString> NMDeviceStateReason;

  private slots:
      void StateChanged(quint32 newState, quint32 oldState, quint32 reason);

  private:
    QString  m_name;
    QString  m_path;
    uint32_t m_index  {0};
    uint32_t m_state  {NM_DEVICE_STATE_UNKNOWN};
    uint32_t m_reason {0};
    bool     m_valid  {false};
};

class MythNetworkDBus : public QObject
{
    Q_OBJECT;

  public:
    MythNetworkDBus(QObject *parent = nullptr);

  private:
    MythNetworkDevice *findByPath (const QString& path);
    MythNetworkDevice *findByIndex (uint32_t index);
    void printChildList (void);

  private slots:
      void DeviceAdded(const QDBusObjectPath& dPath);
      void DeviceRemoved(const QDBusObjectPath& dPath);

  private:
    QDBusInterface *m_networkInterface {nullptr};
};

#endif // MYTHNETWORKDBUS_H
