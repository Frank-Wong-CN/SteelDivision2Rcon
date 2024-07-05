#ifndef NETWORK_RUNNER_H
#define NETWORK_RUNNER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QMap>

#include <rcon.h>

#include "notification_delegate.h"

enum NRState {
    NRIdle,
    NRConnecting,
    NRConnected,
    // NRWaitReply, // Use blocking call instead
    NRReconnecting
};

class NetworkRunner : public QObject
{
    Q_OBJECT
    QAtomicInteger<bool> m_StopToken;

    QString m_Addr;
    QString m_Pass;
    uint16_t m_Port;

    NRState m_State;
    QMutex m_Mutex;
    QTimer m_PollingTimer;

    // QVector<QString> m_CommandBuffer;

    RconClient *m_pClient;
    QMap<uint64_t, QString> m_Clients;

    const uint32_t m_ClientQueryWaitTime = 60; // Query clients per 2 secs
    uint32_t m_CurClientQueryWaitTime = 0;

    const QList<QString> RconSyncVariablesSkipList {
        "ServerName", "Password"
    };

    void CheckConnectivity();
    void ParseClientList(const std::string &data);

public:
    explicit NetworkRunner(QObject *parent = nullptr);
    void Stop();

public slots:
    DECLARE_NOTIFY(msg);

    void StartPolling();
    void Poll();
};

#endif // NETWORK_RUNNER_H
