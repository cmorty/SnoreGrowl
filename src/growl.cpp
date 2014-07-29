#include <growl.hpp>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

GrowlNotificationData::GrowlNotificationData(const Growl *parent, const std::string &notification, const int id, const std::string &title, const std::string &message):
    parent(parent),
    notification(notification),
    id(id),
    title(title),
    message(message),
    icon_data(NULL)
{

}

GrowlNotificationData::~GrowlNotificationData()
{
    if(icon_data)
    {
        delete [] icon_data;
    }
}

void GrowlNotificationData::setIcon(const std::string &icon)
{
    this->icon = icon;
}

void GrowlNotificationData::setIconData(const char *icon_data, const size_t icon_data_size)
{
    this->icon_data = new char[icon_data_size];
    memcpy(this->icon_data,icon_data,icon_data_size);
}

void GrowlNotificationData::setUrl(const std::string &url)
{
    this->url = url;
}

void GrowlNotificationData::setCallbackData(const std::string &data)
{
    this->callback_data = data;
}

growl_notification_data GrowlNotificationData::data() const
{
    growl_notification_data d;
    memset(&d,0,sizeof(growl_notification_data));
    d.app_name = parent->application.c_str();
    d.notify = notification.c_str();
    d.id = id;
    d.title = title.c_str();
    d.message = message.c_str();
    if(!icon_data)
    {
        d.icon = icon.c_str();
    }
    else
    {
        d.icon_data = icon_data;
        d.icon_data_size = icon_data_size;
    }
    if(!url.empty())
    {
        d.url = url.c_str();
    }
    else if(!callback_data.empty())
    {
        d.callback_context = callback_data.c_str();
    }
    return d;
}

Growl::Growl(
        const Growl_Protocol _protocol,
        const std::string &_password,
        const std::string &_application,
        std::vector<std::string> &notifications)
    : server("localhost"),
      password(_password),
      protocol(_protocol), application(_application) {

    Register(notifications);
}

Growl::Growl(
        const Growl_Protocol _protocol,
        const std::string &_server,
        const std::string &_password,
        const std::string &_application,
        std::vector<std::string> &notifications)
    : server(_server),
      password(_password),
      protocol(_protocol), application(_application) {

    Register(notifications);
}

void
Growl::Register(std::vector<std::string> &notifications , const std::string &icon) {

    const char *notify[notifications.size()];
    for(size_t i = 0; i < notifications.size();++i)
    {
        notify[i] = notifications[i].c_str();
    }
    if (protocol == GROWL_TCP) {
        growl_tcp_register(
                    server.c_str(),
                    application.c_str(),
                    notify,
                    notifications.size(),
                    password.c_str(),
                    icon.c_str());
    } else if (protocol == GROWL_UDP) {
        growl_udp_register(
                    server.c_str(),
                    application.c_str(),
                    notify,
                    notifications.size(),
                    password.c_str());
    }
}

Growl::~Growl() {
}


void
Growl::Notify(const GrowlNotificationData &notification) {
    growl_notification_data d = notification.data();
    if (protocol == GROWL_TCP) {
        growl_tcp_notify(server.c_str(), password.c_str(), &d);
    } else if (protocol == GROWL_UDP) {
        growl_udp_notify( server.c_str(), password.c_str(), &d);
    }
}

void Growl::setCallback(GROWL_CALLBACK callback)
{
    growl_set_callback(callback);
}

/* vim:set et sw=2 ts=2 ai: */
