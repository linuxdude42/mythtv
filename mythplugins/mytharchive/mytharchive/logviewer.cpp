#include <chrono>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

// qt
#include <QCoreApplication>
#include <QFile>
#include <QKeyEvent>
#include <QTextStream>

// mythtv
#include <libmythbase/mythcorecontext.h>
#include <libmythbase/mythdbcon.h>
#include <libmythbase/mythlogging.h>
#include <libmythui/mythdialogbox.h>
#include <libmythui/mythmainwindow.h>
#include <libmythui/mythuibutton.h>
#include <libmythui/mythuibuttonlist.h>
#include <libmythui/mythuitext.h>

// mytharchive
#include "archiveutil.h"
#include "logviewer.h"

void showLogViewer(void)
{
    MythScreenStack *mainStack = GetMythMainWindow()->GetMainStack();
    QString logDir = getTempDirectory() + "logs";
    QString progressLog;
    QString fullLog;

    // wait for a log file to be available
    int tries = 10;
    while (tries--)
    {
        if (QFile::exists(logDir + "/progress.log"))
            progressLog = logDir + "/progress.log";

        if (QFile::exists(logDir + "/mythburn.log"))
            fullLog = logDir + "/mythburn.log";

        // we wait for both the progress.log and mythburn.log
        if ((!progressLog.isEmpty()) && (!fullLog.isEmpty()))
            break;

        // or we wait for a log from mytharchivehelper
        if (progressLog.isEmpty() && fullLog.isEmpty())
        {
            QStringList logFiles;
            QStringList filters;
            filters << "*.log";

            QDir d(logDir);
            logFiles = d.entryList(filters, QDir::Files | QDir::Readable, QDir::Time);

            if (!logFiles.isEmpty())
            {
                // the first log file should be the newest one available
                progressLog = logDir + '/' + logFiles[0];
                break;
            }
        }

        sleep(1);
    }

    // do any logs exist?
    if ((!progressLog.isEmpty()) || (!fullLog.isEmpty()))
    {
        auto *viewer = new LogViewer(mainStack);
        viewer->setFilenames(progressLog, fullLog);
        if (viewer->Create())
            mainStack->AddScreen(viewer);
    }
    else
    {
        showWarningDialog(QCoreApplication::translate("LogViewer",
            "Cannot find any logs to show!"));
    }
}

LogViewer::LogViewer(MythScreenStack *parent)
    : MythScreenType(parent, "logviewer"),
      m_autoUpdate(gCoreContext->GetBoolSetting("LogViewerAutoUpdate", true)),
      m_updateTime(gCoreContext->GetDurSetting<std::chrono::seconds>(
                       "LogViewerUpdateTime", DEFAULT_UPDATE_TIME))
{
}

LogViewer::~LogViewer(void)
{
    gCoreContext->SaveDurSetting("LogViewerUpdateTime", m_updateTime);
    gCoreContext->SaveSetting("LogViewerAutoUpdate", m_autoUpdate ? "1" : "0");
    delete m_updateTimer;
}

bool LogViewer::Create(void)
{
    // Load the theme for this screen
    bool foundtheme = LoadWindowFromXML("mytharchive-ui.xml", "logviewer", this);
    if (!foundtheme)
        return false;

    bool err = false; 
    UIUtilE::Assign(this, m_logList, "loglist", &err);
    UIUtilE::Assign(this, m_logText, "logitem_text", &err);
    UIUtilE::Assign(this, m_cancelButton, "cancel_button", &err);
    UIUtilE::Assign(this, m_updateButton, "update_button", &err);
    UIUtilE::Assign(this, m_exitButton, "exit_button", &err);

    if (err)
    {
        LOG(VB_GENERAL, LOG_ERR, "Cannot load screen 'logviewer'");
        return false;
    }

    connect(m_cancelButton, &MythUIButton::Clicked, this, &LogViewer::cancelClicked);
    connect(m_updateButton, &MythUIButton::Clicked, this, &LogViewer::updateClicked);
    connect(m_exitButton, &MythUIButton::Clicked, this, &MythScreenType::Close);

    connect(m_logList, &MythUIButtonList::itemSelected,
            this, &LogViewer::updateLogItem);

    m_updateTimer = nullptr;
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &LogViewer::updateTimerTimeout );

    BuildFocusList();

    SetFocusWidget(m_logList);

    return true;
}

void LogViewer::Init(void)
{
    updateClicked();
    if (m_logList->GetCount() > 0)
        m_logList->SetItemCurrent(m_logList->GetCount() - 1);
}

bool LogViewer::keyPressEvent(QKeyEvent *event)
{
    if (GetFocusWidget()->keyPressEvent(event))
         return true;

    QStringList actions;
    bool handled = GetMythMainWindow()->TranslateKeyPress("Global", event, actions);

    for (int i = 0; i < actions.size() && !handled; i++)
    {
        const QString& action = actions[i];
        handled = true;

        if (action == "MENU")
            ShowMenu();
        else
            handled = false;
    }

    if (!handled && MythScreenType::keyPressEvent(event))
        handled = true;

    return handled;
}

void LogViewer::updateTimerTimeout()
{
    updateClicked();
}

void LogViewer::toggleAutoUpdate(void)
{
    m_autoUpdate = ! m_autoUpdate;

    if (m_autoUpdate)
        m_updateTimer->start(m_updateTime);
    else
        m_updateTimer->stop();
}

void LogViewer::updateLogItem(MythUIButtonListItem *item)
{
    if (item)
        m_logText->SetText(item->GetText());
}

void LogViewer::cancelClicked(void)
{
    QString tempDir = gCoreContext->GetSetting("MythArchiveTempDir", "");
    QFile lockFile(tempDir + "/logs/mythburncancel.lck");

    if (!lockFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        LOG(VB_GENERAL, LOG_ERR,
            "LogViewer: Failed to create mythburncancel.lck file");

    lockFile.write("Cancel\n\r");
    lockFile.close();

    ShowOkPopup(tr("Background creation has been asked to stop.\n" 
                   "This may take a few minutes."));
}

void LogViewer::updateClicked(void)
{
    m_updateTimer->stop();

    QStringList list;
    loadFile(m_currentLog, list, m_logList->GetCount());

    if (!list.empty())
    {
        bool bUpdateCurrent =
                (m_logList->GetCount() == m_logList->GetCurrentPos() + 1) ||
                (m_logList->GetCurrentPos() == 0);

        for (const auto & label : std::as_const(list))
            new MythUIButtonListItem(m_logList, label);

        if (bUpdateCurrent)
            m_logList->SetItemCurrent(m_logList->GetCount() - 1);
    }

    bool bRunning = (getSetting("MythArchiveLastRunStatus") == "Running");

    if (!bRunning)
    {
        m_cancelButton->SetEnabled(false);
        m_updateButton->SetEnabled(false);
    }

    if (m_autoUpdate)
    {
        if (m_logList->GetCount() > 0)
            m_updateTimer->start(m_updateTime);
        else
            m_updateTimer->start(500ms);
    }
}

QString LogViewer::getSetting(const QString &key)
{
    // read the setting direct from the DB rather than from the settings cache 
    // which isn't aware that the script may have changed something
    MSqlQuery query(MSqlQuery::InitCon());
    if (query.isConnected())
    {
        query.prepare("SELECT data FROM settings WHERE value = :VALUE "
                "AND hostname = :HOSTNAME ;");
        query.bindValue(":VALUE", key);
        query.bindValue(":HOSTNAME", gCoreContext->GetHostName());

        if (query.exec() && query.next())
        {
            return query.value(0).toString();
        }
    }
    else
    {
        LOG(VB_GENERAL, LOG_ERR, 
            QString("Database not open while trying to load setting: %1")
                .arg(key));
    }

    return {""};
}

bool LogViewer::loadFile(const QString& filename, QStringList &list, int startline)
{
    bool strip = !(filename.endsWith("progress.log") || filename.endsWith("mythburn.log"));

    list.clear();

    QFile file(filename);

    if (!file.exists())
        return false;

    if (file.open( QIODevice::ReadOnly ))
    {
        QString s;
        QTextStream stream(&file);

         // ignore the first startline lines
        while ( !stream.atEnd() && startline > 0)
        {
            stream.readLine();
            startline--;
        }

         // read rest of file
        while ( !stream.atEnd() )
        {
            s = stream.readLine();

            if (strip)
            {
                // the logging from mytharchivehelper contains a lot of junk
                // we are not interested in so just strip it out
                int pos = s.indexOf(" - ");
                if (pos != -1)
                    s = s.mid(pos + 3);
            }

            list.append(s);
        }
        file.close();
    }
    else
    {
        return false;
    }

    return true;
}

void LogViewer::setFilenames(const QString &progressLog, const QString &fullLog)
{
    m_progressLog = progressLog;
    m_fullLog = fullLog;
    m_currentLog = progressLog;
}

void LogViewer::showProgressLog(void)
{
    m_currentLog = m_progressLog;
    m_logList->Reset();
    updateClicked();
}

void LogViewer::showFullLog(void)
{
    m_currentLog = m_fullLog;
    m_logList->Reset();
    updateClicked();
}

void LogViewer::ShowMenu()
{
    MythScreenStack *popupStack = GetMythMainWindow()->GetStack("popup stack");
    auto *menuPopup = new MythDialogBox(tr("Menu"), popupStack, "actionmenu");

    if (menuPopup->Create())
        popupStack->AddScreen(menuPopup);

    menuPopup->SetReturnEvent(this, "action");

    if (m_autoUpdate)
        menuPopup->AddButton(tr("Don't Auto Update"), &LogViewer::toggleAutoUpdate);
    else
        menuPopup->AddButton(tr("Auto Update"), &LogViewer::toggleAutoUpdate);

    menuPopup->AddButton(tr("Show Progress Log"), &LogViewer::showProgressLog);
    menuPopup->AddButton(tr("Show Full Log"), &LogViewer::showFullLog);
}
