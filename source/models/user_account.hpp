#pragma once
#ifndef AGROMASTER_MODELS_USER_ACCOUNT_HPP_
#define AGROMASTER_MODELS_USER_ACCOUNT_HPP_

#include <string>

#include <Wt/Dbo/Dbo.h>
#include <Wt/WGlobal.h>

namespace agromaster
{
namespace models
{

struct user_account
{
    enum class role
    {
        visitor = 0,
        admin = 1
    };

    user_account() noexcept = default;
    explicit user_account(enum class role new_role) noexcept : role(new_role) {}

    enum class role role = role::visitor;

    template<class Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, role, "role");
    }
};

using AuthInfo = Wt::Auth::Dbo::AuthInfo<agromaster::models::user_account>;

} // models
} // agromaster

#endif // AGROMASTER_MODELS_USER_ACCOUNT_HPP_