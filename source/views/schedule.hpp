#pragma once
#ifndef AGROMASTER_SCHEDULE_HPP_
#define AGROMASTER_SCHEDULE_HPP_

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WImage.h>

namespace agromaster
{

class schedule : public Wt::WContainerWidget
{
public:
    schedule();

private:
    Wt::WText* title_;
};

} // agromaster

#endif // AGROMASTER_SCHEDULE_HPP_