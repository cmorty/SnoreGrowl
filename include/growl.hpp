#ifndef GROWLXX_HPP_
#define GROWLXX_HPP_

#include "growl.h"
#include <string>
#include <vector>

enum Growl_Protocol { GROWL_UDP , GROWL_TCP };

class Growl;

class GROWL_CPP_EXPORT GrowlNotificationData {
private:
    friend class Growl;

    growl_notification_data data(const Growl* sender) const;

    std::string notification;
    int id;
    std::string title;
    std::string message;
    std::string url;
    std::string icon;

    char *icon_data;
    size_t icon_data_size;

    std::string callback_data;

public:
    GrowlNotificationData(const std::string &notification, const int id, const std::string &title, const std::string &message);
    ~GrowlNotificationData();

    void setIcon(const std::string& icon);
    void setIconData(const char *icon_data, const size_t icon_data_size);
    void setUrl(const std::string& url);
    void setCallbackData(const std::string& data);




};

class GROWL_CPP_EXPORT Growl {
private:
    std::string m_server;
    std::string m_password;
    Growl_Protocol m_protocol;
    std::string m_application;

public:
    Growl(const Growl_Protocol protocol, const std::string &password, const std::string &application);
    Growl(const Growl_Protocol protocol, const std::string &server, const std::string &password, const std::string &application);
    ~Growl();

    void Register(const std::vector<std::string> &notifications, const std::string &icon = std::string());

    void Notify(const GrowlNotificationData &notification);

    const std::string &application() const;

    const std::string &server() const;

    Growl_Protocol protocol() const;


    static void setCallback(GROWL_CALLBACK callback);

    static bool init();
    static bool shutdown();

    static bool isRunning(const Growl_Protocol protool, const std::string &server = std::string("localhost"));
};

#endif // GROWLXX_HPP_

/* vim:set et sw=2 ts=2 ai: */
