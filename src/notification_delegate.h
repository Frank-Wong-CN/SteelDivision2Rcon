#ifndef NOTIFICATION_DELEGATE_H
#define NOTIFICATION_DELEGATE_H

#include <QObject>

#include "macro.h"

#define DECLARE_NOTIFY(X, ...) void FIRST(__VA_ARGS__)OnNotified (NotificationMessage X)
#define CONNECT_NOTIFY QObject::connect(NotificationDelegate::Get().get(), SIGNAL(Broadcast(NotificationMessage)), this, SLOT(OnNotified(NotificationMessage)), Qt::QueuedConnection);
#define SEND_NOTIFY NotificationDelegate::Get()->Send

enum MessageType: uint64_t {
    LogMessage = 10000,
    ConnectRconServer = 20001,
    ConnectedRconServer = 20002,
    DisconnectedRconServer = 20003,
    RconRequest = 20100,
    RconRequestDone = 20101,
    RconPlayerRequest = 20102,
    RconPlayerRequestDone = 20103,
    RconMapRequest = 20104,
    RconSyncVariables = 20105,
    RconSetAIDifficulty = 20106,
    RconSetDeck = 20107,
    RconKickPlayer = 20108,
};

/* 1024 bytes */
struct NotificationMessage {
    uint64_t m_MessageID;
    MessageType m_MessageType;
    void *m_Sender = NULL;
    char m_Buffer[1000] = { 0 };
};

Q_DECLARE_METATYPE(NotificationMessage);

class NotificationDelegate : public QObject
{
    Q_OBJECT
    explicit NotificationDelegate(QObject *parent = nullptr);

public:
    static std::shared_ptr<NotificationDelegate> Get();
    void Send(NotificationMessage msg);

signals:
    void Broadcast(NotificationMessage msg);
};

#endif // NOTIFICATION_DELEGATE_H
