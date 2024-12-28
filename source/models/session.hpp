#pragma once
#ifndef AGROMASTER_MODELS_SESSION_HPP_
#define AGROMASTER_MODELS_SESSION_HPP_

#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/SqlConnectionPool.h>

#include "hothouse.hpp"
#include "schedules.hpp"
#include "crop.hpp"
#include "user_account.hpp"

namespace agromaster
{
namespace models
{

using UserDatabase = Wt::Auth::Dbo::UserDatabase<agromaster::models::AuthInfo>;

class session : public Wt::Dbo::Session
{
public:
    explicit session(Wt::Dbo::SqlConnectionPool& connection_pool)
        : users_(std::make_unique<UserDatabase>(*this))
    {
        setConnectionPool(connection_pool);
        mapClass<agromaster::models::user_account>("user_account");
        mapClass<agromaster::models::AuthInfo>("auth_info");
        mapClass<agromaster::models::AuthInfo::AuthIdentityType>("auth_identity");
        mapClass<agromaster::models::AuthInfo::AuthTokenType>("auth_token");
        mapClass<agromaster::models::crop>("crop");
        mapClass<agromaster::models::fertilizer_schedules>("fertilizer_schedules");
        mapClass<agromaster::models::watering_schedules>("watering_schedules");
        mapClass<agromaster::models::schedules>("schedules");
        mapClass<agromaster::models::hothouse>("hothouse");
    }

    Wt::Dbo::ptr<user_account> user() const;
    UserDatabase& users() { return *users_; };
    Wt::Auth::Login& login() { return login_; }

    static void configure_auth();
    static const Wt::Auth::AuthService& auth();
    static const Wt::Auth::PasswordService& password_auth();

private:
    std::unique_ptr<UserDatabase> users_;
    Wt::Auth::Login login_;
};

} // models
} // agromaster

#endif // AGROMASTER_MODELS_SESSION_HPP_