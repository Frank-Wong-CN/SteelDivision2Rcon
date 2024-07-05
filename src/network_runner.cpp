#include "network_runner.h"

#include "config.h"
#include "controller.h"
#include "notification_delegate.h"
#include "server_variables_model.h"

#include <QMap>
#include <QString>

#include <regex>
#include <sstream>

#define Config Config::Get()

// =================================================================================================
//         HELPERS
// =================================================================================================

void NetworkRunner::CheckConnectivity() {
    if (!m_pClient) return;
    if (m_State == NRIdle ||
        m_State == NRConnecting ||
        m_State == NRReconnecting) return;

    if (!m_pClient->IsConnected()) {
        Log::AddLog("[WARNING] Rcon connection dropped. Retrying...");
        SEND_NOTIFY({ 0, DisconnectedRconServer });
        m_State = NRReconnecting;
    }
}

void NetworkRunner::ParseClientList(const std::string &data) {
    m_Clients.clear();
    std::istringstream iss;
    int i = -1;
    iss.str(data);
    const std::regex clientInfoRegex("([0-9]+)\\s(.+)\\s?");
    for (std::string curstr; std::getline(iss, curstr); ) {
        if (-1 == i) {
            i += 1; continue; // Skip the line "Client list :"
        }
        if (curstr.starts_with("Server is empty !")) return;
        std::smatch clientInfo;
        std::regex_match(curstr, clientInfo, clientInfoRegex);
        if (clientInfo.size() == 3) {
            uint64_t id = std::atoll(clientInfo[1].str().c_str());
            m_Clients.insert(id, clientInfo[2].str().c_str());
            // Log::AddLog(QString("Client: %1 (%2)").arg(clientInfo[2].str().c_str(), id));
        }
    }
}

// =================================================================================================
//         EVENT PROC
// =================================================================================================

DECLARE_NOTIFY(msg, NetworkRunner::) {
    if (msg.m_MessageType == LogMessage) return;
    if (msg.m_MessageType == RconRequestDone) return;
    if (m_State != NRConnected && msg.m_MessageType != ConnectRconServer) {
        Log::AddLog("[ERROR] Connect a server first!");
        SEND_NOTIFY({ 0, RconRequestDone });
        return;
    }
    switch (msg.m_MessageType) {
    case ConnectRconServer: {
        // Place this one after the mutex could potentially trigger a reconnect after a successful connect
        if (m_State == NRConnecting) break;

        QMutexLocker<QMutex> _guard(&m_Mutex);
        // Only after a full tick should the state be changed, in case the state prematurally changed during the tick
        m_State = NRConnecting;
        break;
    }
    case RconPlayerRequest: {
        auto &resp = m_pClient->Send("display_all_clients");
        resp.WaitData();
        if (resp.Responded()) {
            // Parse client list
            ParseClientList(resp.Data());
            SEND_NOTIFY({ 0, RconPlayerRequestDone, &m_Clients });
        }
        m_pClient->DoneWithMessage(resp);
        break;
    }
    case RconSyncVariables:
    case RconRequest: {
        ServerVariables *var = (ServerVariables *)msg.m_Sender;
        for (const ServerVariableItemData &ref : *var) {
            if (msg.m_MessageType == RconRequest && !ref.m_Selected) continue;
            if (msg.m_MessageType == RconSyncVariables && RconSyncVariablesSkipList.contains(ref.m_Varname)) continue;
            QString value;
            if (ref.m_Value.typeId() == qMetaTypeId<ValueOption>()) {
                const ValueOption valopt = ref.m_Value.value<ValueOption>();
                value = valopt.m_Value.toString();
            }
            else {
                value = ref.m_Value.toString();
            }
            QString command = QString("setsvar %1 %2").arg(ref.m_Varname, value);
            Log::AddLog(QString("Sending: %1").arg(command));
            auto &resp = m_pClient->Send(command.toStdString());
            resp.WaitData();
            if (resp.Responded()) {
                Log::AddLog(resp.Data().c_str());
            }
            m_pClient->DoneWithMessage(resp);
        }
        SEND_NOTIFY({ 0, RconRequestDone });
        break;
    }
    case RconMapRequest: {
        QString *suffix = (QString *)msg.m_Sender;
        QString command = QString("setsvar Map %1%2").arg(Config->GetMap(), *suffix);
        Log::AddLog(QString("Sending: %1").arg(command));
        auto &resp = m_pClient->Send(command.toStdString());
        resp.WaitData();
        if (resp.Responded()) {
            Log::AddLog(resp.Data().c_str());
        }
        SEND_NOTIFY({ 0, RconRequestDone });
        break;
    }
    case RconSetDeck:
    case RconSetAIDifficulty: {
        QString command = QString("setpvar %1").arg(msg.m_Buffer);
        Log::AddLog(QString("Sending: %1").arg(command));
        auto &resp = m_pClient->Send(command.toStdString());
        resp.WaitData();
        if (resp.Responded()) {
            Log::AddLog(resp.Data().c_str());
        }
        break;
    }
    default:
        break;
    }
}

// =================================================================================================
//         STATE MACHINE
// =================================================================================================

NetworkRunner::NetworkRunner(QObject *parent)
    : QObject{parent}, m_State(NRIdle), m_PollingTimer(this), m_pClient(nullptr)
{
    CONNECT_NOTIFY;
    QObject::connect(&m_PollingTimer, SIGNAL(timeout()), this, SLOT(Poll()));
}

void NetworkRunner::Stop() {
    // Log::AddLog("Network runner stop requested.");
    m_StopToken.storeRelaxed(true);
}

void NetworkRunner::StartPolling() {
    m_PollingTimer.start(NETWORK_POLLING_RATE);
}

void NetworkRunner::Poll() {
    // Log::AddLog("Network runner started.");

    if (m_StopToken.loadRelaxed()) {
        m_PollingTimer.stop();
        if (m_pClient) delete m_pClient;
        QThread::currentThread()->exit(0);
    }

    QMutexLocker<QMutex> _guard(&m_Mutex);
    CheckConnectivity();

    switch (m_State) {
    case NRConnecting:
        // Only update the connection detail when specifically instructed by the user
        m_Addr = Config->GetRconAddress();
        m_Pass = Config->GetRconPassword();
        m_Port = Config->GetRconPort();

    case NRReconnecting: {
        Log::AddLog(QString("Connecting to server at %1...").arg(m_Addr));

        if (m_pClient) delete m_pClient;

        m_pClient = new RconClient(m_Addr.toStdString(), m_Port, m_Pass.toStdString(), [](const std::string_view &log) {
            Log::AddLog(log.data());
        });
        if (!m_pClient->IsConnected()) {
            Log::AddLog("[ERROR] Cannot connect to the Rcon server.");
            SEND_NOTIFY({ 0, DisconnectedRconServer });
            m_State = NRReconnecting;
            return;
        }
        Log::AddLog("Connected!");
        SEND_NOTIFY({ 0, ConnectedRconServer });
        m_State = NRConnected;
        break;
    }
    case NRConnected:
        ++m_CurClientQueryWaitTime;
        if (m_CurClientQueryWaitTime >= m_ClientQueryWaitTime) {
            SEND_NOTIFY({ 0, RconPlayerRequest });
            m_CurClientQueryWaitTime = 0;
        }

    default:
    case NRIdle:
        break;
    }
}
