#pragma once
#ifndef AGROMASTER_MODELS_WORKS_HPP_
#define AGROMASTER_MODELS_WORKS_HPP_

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

namespace agromaster
{
namespace models
{

struct hothouse;
struct works;

struct fertilizer_works
{
    fertilizer_works() = default;
    explicit fertilizer_works(const Wt::WDate& d) : date(d) {}

    Wt::WDate date;
    Wt::Dbo::ptr<works> works;

    template <typename Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, date, "date");
        Wt::Dbo::belongsTo(action, works, "works",
            Wt::Dbo::ForeignKeyConstraint(Wt::Dbo::NotNull | Wt::Dbo::OnUpdateCascade | Wt::Dbo::OnDeleteCascade));
    }
};

struct watering_works
{
    watering_works() = default;
    explicit watering_works(const Wt::WDate& d) : date(d) {}

    Wt::WDate date;
    Wt::Dbo::ptr<works> works;

    template <typename Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, date, "date");
        Wt::Dbo::belongsTo(action, works, "works",
            Wt::Dbo::ForeignKeyConstraint(Wt::Dbo::NotNull | Wt::Dbo::OnUpdateCascade | Wt::Dbo::OnDeleteCascade));
    }
};

struct works
{
    Wt::WDate sowing_work;
    Wt::WDate harvest_work;
    Wt::Dbo::collection<Wt::Dbo::ptr<fertilizer_works>> fertilizer_works;
    Wt::Dbo::collection<Wt::Dbo::ptr<watering_works>> watering_works;
    Wt::Dbo::ptr<hothouse> hothouse;

    template <typename Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, sowing_work, "sowing_work");
        Wt::Dbo::field(action, harvest_work, "harvest_work");
        Wt::Dbo::hasMany(action, fertilizer_works, Wt::Dbo::ManyToOne, "works");
        Wt::Dbo::hasMany(action, watering_works, Wt::Dbo::ManyToOne, "works");
        Wt::Dbo::belongsTo(action, hothouse, "hothouse",
            Wt::Dbo::ForeignKeyConstraint(Wt::Dbo::NotNull | Wt::Dbo::OnUpdateCascade | Wt::Dbo::OnDeleteCascade));
    }
};

} // models
} // agromaster

#endif // AGROMASTER_MODELS_WORKS_HPP_