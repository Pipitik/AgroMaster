#pragma once
#ifndef AGROMASTER_MODELS_CROP_HPP_
#define AGROMASTER_MODELS_CROP_HPP_

#include <Wt/Dbo/Dbo.h>

#include "hothouse.hpp"
#include "schedules.hpp"

namespace agromaster
{
namespace models
{

struct crop
{
    std::string title;
    Wt::Dbo::collection<Wt::Dbo::ptr<hothouse>> hothouses;
    Wt::Dbo::weak_ptr<schedules> schedules;

    template <typename Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, title, "title", 30);
        Wt::Dbo::hasMany(action, hothouses, Wt::Dbo::ManyToOne, "crop");
        Wt::Dbo::hasOne(action, schedules);
    }
};

} // models
} // agromaster

#endif // AGROMASTER_MODELS_CROP_HPP_