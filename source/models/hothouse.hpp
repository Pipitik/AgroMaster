#pragma once
#ifndef AGROMASTER_MODELS_HOTHOUSE_HPP_
#define AGROMASTER_MODELS_HOTHOUSE_HPP_

#include <Wt/Dbo/Dbo.h>

namespace agromaster
{
namespace models
{

struct crop;

struct hothouse
{
    std::string title;
    double yields;
    double spent_fertilizers;
    Wt::Dbo::ptr<crop> crop;

    template <typename Action>
    void persist(Action& action)
    {
        Wt::Dbo::field(action, title, "title", 30);
        Wt::Dbo::field(action, yields, "yields");
        Wt::Dbo::field(action, spent_fertilizers, "spent_fertilizers");
        Wt::Dbo::belongsTo(action, crop, "crop",
            Wt::Dbo::ForeignKeyConstraint(Wt::Dbo::OnUpdateCascade | Wt::Dbo::OnDeleteSetNull));
    }
};

} // models
} // agromaster

#endif // AGROMASTER_MODELS_HOTHOUSE_HPP_