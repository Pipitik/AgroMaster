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

class application : public Wt::WApplication
{
public:
    application(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connection_pool);

private:
    void set_auth_widget();
    void set_navigation_bar(const Wt::WString& login_name);
    void handle_path_changes();
    void set_hothouses_table();
    void set_crop_table();

    void add_hothouse_row(Wt::WTable& table, int index, const agromaster::models::hothouse& hothouse);
    void add_crop_row(Wt::WTable& table, int index, const agromaster::models::crop& crop);

    void show_dialog_add_hothouse();

    void show_dialog_add_crop();
    void show_dialog_change_crop(Wt::WText* crop_title);
    void handle_delete_crop(const Wt::WTableRow* row, const Wt::WString& crop_title);

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