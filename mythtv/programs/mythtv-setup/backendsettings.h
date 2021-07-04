#ifndef BACKENDSETTINGS_H
#define BACKENDSETTINGS_H

#include "standardsettings.h"

class IpAddressSettings;
class BackendSettings : public GroupSetting
{
  Q_OBJECT
  public:
    BackendSettings();
    void Load(void) override; // StandardSetting
    void Save(void) override; // StandardSetting
    ~BackendSettings() override;

  private:
    TransMythUICheckBoxSetting *m_isPrimaryBackend  {nullptr};
    HostTextEditSetting        *m_localServerPort   {nullptr};
    HostComboBoxSetting        *m_backendServerAddr {nullptr};
    GlobalTextEditSetting      *m_primaryServerName {nullptr};
    IpAddressSettings          *m_ipAddressSettings {nullptr};
    bool                        m_isLoaded          {false};
    QString                     m_priorPrimaryName;

    // Deprecated - still here to support bindings
    GlobalTextEditSetting      *m_primaryServerIP   {nullptr};
    GlobalTextEditSetting      *m_primaryServerPort {nullptr};

  private slots:
    void primaryBackendChanged(void);
    void listenChanged(void);
};

#endif

