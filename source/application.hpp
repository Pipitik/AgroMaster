#pragma once
#ifndef AGROMASTER_APPLICATION_HPP_
#define AGROMASTER_APPLICATION_HPP_

#include <Wt/Auth/AuthWidget.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WServer.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>

#include "models.hpp"

namespace agromaster
{ 

namespace internal_path
{
static constexpr char root[] = "/";
static constexpr char hothouses[] = "/hothouses/";
static constexpr char crops[] = "/crops/";
} // internal_path

class application final : public Wt::WApplication
{
public:
    application(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connection_pool);

private:
    void handle_path_changes();
    void set_navigation_bar(const Wt::WString& login_name);
    
    void add_hothouse_row(Wt::WTable& table, int index, const agromaster::models::hothouse& hothouse);
    void set_hothouses_table();    
    void update_hothouses_table();
    void show_dialog_add_hothouse();
    void handle_add_hothouse(const std::string& title, const std::string& crop_title);
    void show_dialog_change_hothouse(
        Wt::WText* current_title,
        Wt::WText* current_crop_title,
        Wt::WText* current_yields,
        Wt::WText* current_spent_fertilizers);
    void handle_change_hothouse(
        const std::string& new_title,
        const std::string& new_crop,
        const std::string& new_yields,
        const std::string& new_spent_fertilizers,
        Wt::WText* current_title,
        Wt::WText* current_crop_title,
        Wt::WText* current_yields,
        Wt::WText* current_spent_fertilizers);
    void show_dialog_hothouse_works(Wt::WText* title);
    void handle_change_hothouse_works(
        const std::string& title,
        const Wt::WDate& sowing_date,
        const Wt::WDate& harvest_date,
        const std::set<Wt::WDate>& fertilizer_dates,
        const std::set<Wt::WDate>& watering_dates);
    void handle_delete_hothouse(const Wt::WTableRow* row, const Wt::WString& hothouse_title);

    void add_crop_row(Wt::WTable& table, int index, const agromaster::models::crop& crop);
    void set_crops_table();
    void update_crops_table();
    void show_dialog_add_crop();
    void handle_add_crop(const std::string& title);
    void show_dialog_change_crop(Wt::WText* title);
    void show_dialog_crop_schedules(Wt::WText* title);
    void handle_change_crop_schedules(
        const std::string& title,
        const Wt::WDate& sowing_date,
        const Wt::WDate& harvest_date,
        const std::set<Wt::WDate>& fertilizer_dates,
        const std::set<Wt::WDate>& watering_dates);
    void handle_delete_crop(const Wt::WTableRow* row, const Wt::WString& crop_title);

    void set_auth_widget();
    void handle_auth();

    models::session db_session_;
    Wt::Auth::AuthWidget* auth_widget_ = nullptr;
    Wt::WNavigationBar* navigation_ = nullptr;
    Wt::WStackedWidget* main_stack_ = nullptr;
    Wt::WContainerWidget* hothouses_ = nullptr;
    Wt::WContainerWidget* crops_ = nullptr;
    enum class models::user_account::role user_role_ = models::user_account::role::visitor;
};

} // agromaster

#endif // AGROMASTER_APPLICATION_HPP_