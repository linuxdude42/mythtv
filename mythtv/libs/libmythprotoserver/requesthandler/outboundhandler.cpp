#include <chrono>

#include <QTimer>
#include <QString>
#include <QStringList>

#include "mythsocket.h"
#include "mythsocketmanager.h"
#include "socketrequesthandler.h"
#include "sockethandler.h"
#include "mythlogging.h"
#include "mythcorecontext.h"
#include "compat.h"

#include "requesthandler/outboundhandler.h"

OutboundRequestHandler::OutboundRequestHandler(void) 
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &OutboundRequestHandler::ConnectToPrimary);
}

void OutboundRequestHandler::ConnectToPrimary(void)
{
    m_timer.stop();
    if (!DoConnectToPrimary())
        m_timer.start(5s);
}

bool OutboundRequestHandler::DoConnectToPrimary(void)
{
    if (m_socket)
        m_socket->DecrRef();

    m_socket = new MythSocket(-1, m_parent);

    QString server   = gCoreContext->GetPrimaryServerIP();
    QString hostname = gCoreContext->GetPrimaryHostName();
    int port         = MythCoreContext::GetPrimaryServerPort();

    if (!m_socket->ConnectToHost(server, port))
    {
        LOG(VB_GENERAL, LOG_ERR, "Failed to connect to primary backend.");
        m_socket->DecrRef();
        m_socket = nullptr;
        return false;
    }

#ifndef IGNORE_PROTO_VER_MISMATCH
    if (!m_socket->Validate())
    {
        LOG(VB_GENERAL, LOG_NOTICE, "Unable to confirm protocol version with backend.");
        m_socket->DecrRef();
        m_socket = nullptr;
        return false;
    }
#endif

    if (!AnnounceSocket())
    {
        LOG(VB_GENERAL, LOG_NOTICE, "Announcement to upstream primary backend failed.");
        m_socket->DecrRef();
        m_socket = nullptr;
        return false;
    }

    auto *handler = new SocketHandler(m_socket, m_parent, hostname);
    handler->BlockShutdown(true);
    handler->AllowStandardEvents(true);
    handler->AllowSystemEvents(true);
    m_parent->AddSocketHandler(handler); // register socket for reception of events
    handler->DecrRef(); // drop local instance in counter
    handler = nullptr;

    LOG(VB_GENERAL, LOG_NOTICE, "Connected to primary backend.");

    return true;
}

void OutboundRequestHandler::connectionClosed(MythSocket *socket)
{
    // connection has closed, trigger an immediate reconnection
    if (socket == m_socket)
        ConnectToPrimary();
}
