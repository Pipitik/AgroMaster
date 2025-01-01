#pragma once
#ifndef AGROMASTER_MODELS_SCHEDULES_HPP_
#define AGROMASTER_MODELS_SCHEDULES_HPP_

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

namespace agromaster
{
namespace models
{

struct crop;
struct schedules;

struct fertilizer_schedules
{
    fertilizer_schedules() = default;
    explicit fertilizer_schedules(const Wt::WDate& d) : date(d) {}

    Wt::WDate date;
    Wt::Dbo::ptr<schedules> schedules;

    template <typename Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, date, "date");
        Wt::Dbo::belongsTo(action, schedules, "schedules",
            Wt::Dbo::ForeignKeyConstraint(Wt::Dbo::NotNull | Wt::Dbo::OnUpdateCascade | Wt::Dbo::OnDeleteCascade));
    }
};

struct watering_schedules
{
    watering_schedules() = default;
    explicit watering_schedules(const Wt::WDate& d) : date(d) {}

    Wt::WDate date;
    Wt::Dbo::ptr<schedules> schedules;

    template <typename Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, date, "date");
        Wt::Dbo::belongsTo(action, schedules, "schedules",
            Wt::Dbo::ForeignKeyConstraint(Wt::Dbo::NotNull | Wt::Dbo::OnUpdateCascade | Wt::Dbo::OnDeleteCascade));
    }
};

struct schedules
{
    Wt::WDate sowing_schedule;
    Wt::WDate harvest_schedule;
    Wt::Dbo::collection<Wt::Dbo::ptr<fertilizer_schedules>> fertilizer_schedules;
    Wt::Dbo::collection<Wt::Dbo::ptr<watering_schedules>> watering_schedules;
    Wt::Dbo::ptr<crop> crop;

    template <typename Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, sowing_schedule, "sowing_schedule");
        Wt::Dbo::field(action, harvest_schedule, "harvest_schedule");
        Wt::Dbo::hasMany(action, fertilizer_schedules, Wt::Dbo::ManyToOne, "schedules");
        Wt::Dbo::hasMany(action, watering_schedules, Wt::Dbo::ManyToOne, "schedules");
        Wt::Dbo::belongsTo(action, crop, "crop",
            Wt::Dbo::ForeignKeyConstraint(Wt::Dbo::NotNull | Wt::Dbo::OnUpdateCascade | Wt::Dbo::OnDeleteCascade));
    }
};

} // models
} // agromaster

#endif // AGROMASTER_MODELS_SCHEDULES_HPP_