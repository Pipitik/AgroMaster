#include "session.hpp"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

namespace
{

Wt::Auth::AuthService auth_service;
Wt::Auth::PasswordService password_service(auth_service);

} // unnamed namespace

namespace agromaster
{
namespace models
{

Wt::Dbo::ptr<models::user_account> session::user() const
{
    if (login_.loggedIn()) {
        Wt::Dbo::ptr<AuthInfo> auth_info = users_->find(login_.user());
        return auth_info->user();
    }
    else
    {
        return Wt::Dbo::ptr<models::user_account>();
    }
}

void session::configure_auth()
{
    auto verifier = std::make_unique<Wt::Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(12));
    password_service.setVerifier(std::move(verifier));
    password_service.setStrengthValidator(std::make_unique<Wt::Auth::PasswordStrengthValidator>());
}

const Wt::Auth::AuthService& session::auth()
{
    return auth_service;
}

const Wt::Auth::PasswordService& session::password_auth()
{
    return password_service;
}

} // models
} // agromaster