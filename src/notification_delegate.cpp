#include "notification_delegate.h"

#include <QThread>

NotificationDelegate::NotificationDelegate(QObject *parent)
    : QObject{parent}
{}

std::shared_ptr<NotificationDelegate> NotificationDelegate::Get() {
    static std::shared_ptr<NotificationDelegate> s_Instance(new NotificationDelegate());
    return s_Instance;
}

void NotificationDelegate::Send(NotificationMessage msg) {
    emit Broadcast(msg);
}
