#include "growl.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

GrowlNotificationData::GrowlNotificationData(const std::string &notification, const int id, const std::string &title, const std::string &message):
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

growl_notification_data GrowlNotificationData::data(const Growl *sender) const
{
    growl_notification_data d;
    memset(&d,0,sizeof(growl_notification_data));
    d.app_name = sender->application().c_str();
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

Growl::Growl(const Growl_Protocol protocol, const std::string &password, const std::string &application)
    : m_server("localhost"),
      m_password(password),
      m_protocol(protocol),
      m_application(application)
{

}

Growl::Growl(const Growl_Protocol protocol, const std::string &server, const std::string &password, const std::string &application)
    : m_server(server),
      m_password(password),
      m_protocol(protocol),
      m_application(application)
{

}

void
Growl::Register(const std::vector<std::string> &notifications , const std::string &icon)
{
    const char **notify = new const char*[notifications.size()];
    for(size_t i = 0; i < notifications.size();++i)
    {
        notify[i] = notifications[i].c_str();
    }
    if (m_protocol == GROWL_TCP) {
        growl_tcp_register(
                    m_server.c_str(),
                    m_application.c_str(),
                    notify,
                    notifications.size(),
                    m_password.c_str(),
                    icon.c_str());
    } else if (m_protocol == GROWL_UDP) {
        growl_udp_register(
                    m_server.c_str(),
                    m_application.c_str(),
                    notify,
                    notifications.size(),
                    m_password.c_str());
    }
	delete[] notify;
}

Growl::~Growl() {
}


void
Growl::Notify(const GrowlNotificationData &notification) {
    growl_notification_data d = notification.data(this);
    if (m_protocol == GROWL_TCP) {
        growl_tcp_notify(m_server.c_str(), m_password.c_str(), &d);
    } else if (m_protocol == GROWL_UDP) {
        growl_udp_notify( m_server.c_str(), m_password.c_str(), &d);
    }
}

const std::string &Growl::application() const
{
    return m_application;
}

const std::string &Growl::server() const
{
    return m_server;
}

Growl_Protocol Growl::protocol() const
{
    return m_protocol;
}

void Growl::setCallback(GROWL_CALLBACK callback)
{
    growl_set_callback(callback);
}

bool Growl::init()
{
    return growl_init() == 1;
}

bool Growl::shutdown()
{
    return growl_shutdown() == 1;
}

bool Growl::isRunning(const Growl_Protocol protool, const std::string &server)
{
    if(protool == GROWL_TCP)
    {
        return growl_tcp_is_running(server.c_str()) != -1;
    }
    else
    {
        //TODO: implement
        return true;
    }
}

/* vim:set et sw=2 ts=2 ai: */
