#include "application.hpp"

#include <numeric>

#include <Wt/Auth/PasswordService.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WRegExpValidator.h>

namespace agromaster
{

application::application(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connection_pool)
    : Wt::WApplication(env)
    , db_session_(connection_pool)
{
    setTitle("AgroMaster");
    setTheme(std::make_shared<Wt::WBootstrap5Theme>());

    db_session_.login().changed().connect(this, &application::handle_auth);
    internalPathChanged().connect(this, &application::handle_path_changes);
    set_auth_widget();
    setInternalPath(internal_path::root);
}

void application::handle_path_changes()
{
    if (internalPathMatches(internal_path::hothouses))
    {
        main_stack_->setCurrentWidget(hothouses_);
    }
    else if (internalPathMatches(internal_path::crops))
    {
        main_stack_->setCurrentWidget(crops_);
    }
    else if (db_session_.login().loggedIn())
    {
        setInternalPath(internal_path::hothouses);
    }
    else
    {
        if (!navigation_->isHidden())
        {
            navigation_->hide();
        }
        if (!main_stack_->isHidden())
        {
            main_stack_->hide();
        }
        if (main_stack_->isHidden())
        {
            auth_widget_->show();
        }
        setInternalPath(internal_path::root);
    }
}

void application::set_auth_widget()
{
    auth_widget_ = root()->addNew<Wt::Auth::AuthWidget>(
        models::session::auth(), db_session_.users(), db_session_.login());
    auth_widget_->model()->addPasswordAuth(&models::session::password_auth());
    auth_widget_->setRegistrationEnabled(true);
    auth_widget_->setWidth("50%");
    auth_widget_->setStyleClass("mx-auto justify-content-center");
}

void application::set_navigation_bar(const Wt::WString& login_name)
{
    navigation_ = root()->addNew<Wt::WNavigationBar>();
    navigation_->addStyleClass("navbar-dark bg-success");
    navigation_->setTitle("AgroMaster");
    navigation_->setResponsive(true);

    Wt::WStackedWidget* contents_stack = root()->addNew<Wt::WStackedWidget>();
    contents_stack->addStyleClass("contents");

    auto left_menu = navigation_->addMenu(std::make_unique<Wt::WMenu>(contents_stack));
    left_menu->addItem(u8"Теплицы", nullptr)->setLink(Wt::WLink(Wt::LinkType::InternalPath, internal_path::hothouses));
    left_menu->addItem(u8"Культуры", nullptr)->setLink(Wt::WLink(Wt::LinkType::InternalPath, internal_path::crops));
    left_menu->addStyleClass("me-auto");

    auto login_name_text = std::make_unique<Wt::WText>(login_name);
    login_name_text->setStyleClass("fs-5 text-white me-3");

    auto logout_button = std::make_unique<Wt::WPushButton>(u8"Выйти");

    logout_button->clicked().connect(
        [&session = db_session_]()
    {
        session.login().logout();
    });

    navigation_->addWidget(std::move(login_name_text));
    navigation_->addWidget(std::move(logout_button));
}

void application::set_hothouses_table()
{
    hothouses_ = main_stack_->addNew<Wt::WContainerWidget>();
    
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::collection<Wt::Dbo::ptr<models::hothouse>> hothouses = db_session_.find<models::hothouse>();

    if (user_role_ == models::user_account::role::admin)
    {
        auto add_new_hothouse_button = hothouses_->addNew<Wt::WPushButton>(u8"Добавить");
        add_new_hothouse_button->setStyleClass("m-3");
        add_new_hothouse_button->clicked().connect(this, &application::show_dialog_add_hothouse);
    }

    auto hothouses_table = hothouses_->addNew<Wt::WTable>();
    hothouses_table->addStyleClass("table table-striped");
    hothouses_table->setWidth("100%");
    int i = 0;

    hothouses_table->setHeaderCount(1);
    hothouses_table->elementAt(i, 0)->addNew<Wt::WText>(u8"Название");
    hothouses_table->elementAt(i, 1)->addNew<Wt::WText>(u8"Культура");
    hothouses_table->elementAt(i, 2)->addNew<Wt::WText>(u8"Урожай, кг");
    hothouses_table->elementAt(i, 3)->addNew<Wt::WText>(u8"Затрачено удобрений, кг");
    if (user_role_ == agromaster::models::user_account::role::admin)
    {
        hothouses_table->elementAt(i, 4)->addNew<Wt::WText>("");
        hothouses_table->elementAt(i, 5)->addNew<Wt::WText>("");
    }

    ++i;
    for (const Wt::Dbo::ptr<models::hothouse> hothouse : hothouses)
    {
        add_hothouse_row(*hothouses_table, i, *hothouse);
        ++i;
    }
}

void application::set_crop_table()
{
    crops_ = main_stack_->addNew<Wt::WContainerWidget>();

    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::collection<Wt::Dbo::ptr<models::crop>> crops = db_session_.find<models::crop>();

    if (user_role_ == models::user_account::role::admin)
    {
        auto add_new_crop_button = crops_->addNew<Wt::WPushButton>(u8"Добавить");
        add_new_crop_button->setStyleClass("m-3");
        add_new_crop_button->clicked().connect(this, &application::show_dialog_add_crop);
    }

    auto crops_table = crops_->addNew<Wt::WTable>();
    crops_table->addStyleClass("table table-striped");
    crops_table->setWidth("100%");
    int i = 0;

    crops_table->setHeaderCount(1);
    crops_table->elementAt(i, 0)->addNew<Wt::WText>(u8"Название");
    crops_table->elementAt(i, 1)->addNew<Wt::WText>(u8"Используется теплиц");
    crops_table->elementAt(i, 2)->addNew<Wt::WText>(u8"Урожай, кг");
    crops_table->elementAt(i, 3)->addNew<Wt::WText>(u8"Затрачено удобрений, кг");
    if (user_role_ == agromaster::models::user_account::role::admin)
    {
        crops_table->elementAt(i, 4)->addNew<Wt::WText>("");
        crops_table->elementAt(i, 5)->addNew<Wt::WText>("");
        crops_table->elementAt(i, 6)->addNew<Wt::WText>("");
    }

    ++i;
    for (const Wt::Dbo::ptr<models::crop> crop : crops)
    {
        add_crop_row(*crops_table, i, *crop);
        ++i;
    }
}

void application::add_hothouse_row(Wt::WTable& table, int index, const agromaster::models::hothouse& hothouse)
{
    constexpr char style_class[] = "text-center";
    auto crop_title = table.elementAt(index, 0)->addNew<Wt::WText>(hothouse.title);
    table.elementAt(index, 0)->setStyleClass(style_class);
    table.elementAt(index, 1)->addNew<Wt::WText>(hothouse.crop->title);
    table.elementAt(index, 1)->setStyleClass(style_class);
    table.elementAt(index, 2)->addNew<Wt::WText>(std::to_string(hothouse.yields));
    table.elementAt(index, 2)->setStyleClass(style_class);
    table.elementAt(index, 3)->addNew<Wt::WText>(std::to_string(hothouse.spent_fertilizers));
    table.elementAt(index, 3)->setStyleClass(style_class);
    if (user_role_ == agromaster::models::user_account::role::admin)
    {
        table.elementAt(index, 4)->addNew<Wt::WPushButton>(u8"Изменить")->disable();
        table.elementAt(index, 4)->setStyleClass(style_class);
        table.elementAt(index, 4)->addNew<Wt::WPushButton>(u8"Удалить")->disable();
        table.elementAt(index, 4)->setStyleClass(style_class);
    }
}

void application::add_crop_row(Wt::WTable& table, int index, const agromaster::models::crop& crop)
{
    constexpr char style_class[] = "text-center";
    auto crop_title = table.elementAt(index, 0)->addNew<Wt::WText>(crop.title);
    table.elementAt(index, 0)->setStyleClass(style_class);
    table.elementAt(index, 1)->addNew<Wt::WText>(std::to_string(crop.hothouses.size()));
    table.elementAt(index, 1)->setStyleClass(style_class);
    table.elementAt(index, 2)->addNew<Wt::WText>(
        std::to_string(std::accumulate(crop.hothouses.begin(), crop.hothouses.end(), 0.0,
            [](double value, const Wt::Dbo::ptr<agromaster::models::hothouse>& h)
    {
        return value + h->yields;
    })));
    table.elementAt(index, 2)->setStyleClass(style_class);
    table.elementAt(index, 3)->addNew<Wt::WText>(
        std::to_string(std::accumulate(crop.hothouses.begin(), crop.hothouses.end(), 0.0,
            [](double value, const Wt::Dbo::ptr<agromaster::models::hothouse>& h)
    {
        return value + h->spent_fertilizers;
    })));
    table.elementAt(index, 3)->setStyleClass(style_class);
    if (user_role_ == agromaster::models::user_account::role::admin)
    {
        table.elementAt(index, 4)->addNew<Wt::WPushButton>(u8"График")->disable();
        table.elementAt(index, 4)->setStyleClass(style_class);
        table.elementAt(index, 5)->addNew<Wt::WPushButton>(u8"Изменить")->
            clicked().connect(
                [this, crop_title]()
            {
                show_dialog_change_crop(crop_title);
            });
        table.elementAt(index, 5)->setStyleClass(style_class);
        table.elementAt(index, 6)->addNew<Wt::WPushButton>(u8"Удалить")->
            clicked().connect(
                [this, row = table.rowAt(index), crop_title]()
            {
                handle_delete_crop(row, crop_title->text());
            });
        table.elementAt(index, 6)->setStyleClass(style_class);
    }
}

void application::show_dialog_add_hothouse()
{
    auto dialog = root()->addNew<Wt::WDialog>(u8"Добавить новую теплицу");

    Wt::WLabel* label = dialog->contents()->addNew<Wt::WLabel>(u8"Название");
    Wt::WLineEdit* edit = dialog->contents()->addNew<Wt::WLineEdit>();
    label->setBuddy(edit);

    Wt::WLabel* label = dialog->contents()->addNew<Wt::WLabel>(u8"Культура");
    Wt::WLineEdit* pu = dialog->contents()->addNew<Wt::WLineEdit>();
    label->setBuddy(edit);

    dialog->contents()->addStyleClass("form-group");

    auto validator = std::make_shared<Wt::WRegExpValidator>(u8"[A-Za-z0-9\x0400-\x04ff]{0,30}");
    validator->setMandatory(true);
    edit->setValidator(validator);

    Wt::WPushButton* ok = dialog->footer()->addNew<Wt::WPushButton>(u8"принять");
    ok->setDefault(true);
    if (wApp->environment().ajax())
    {
        ok->disable();
    }

    Wt::WPushButton* cancel = dialog->footer()->addNew<Wt::WPushButton>(u8"отмена");
    dialog->rejectWhenEscapePressed();
    edit->keyWentUp().connect(
        [ok, edit]
    {
        ok->setDisabled(edit->validate() != Wt::ValidationState::Valid);
    });

    ok->clicked().connect(
        [edit, dialog]
    {
        if (edit->validate() == Wt::ValidationState::Valid)
        {
            dialog->accept();
        }
    });

    cancel->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->finished().connect(
        [this, dialog, edit]
    {
        if (dialog->result() == Wt::DialogCode::Accepted)
        {
            Wt::Dbo::Transaction transaction(db_session_);
            auto new_hothouse = db_session_.addNew<models::hothouse>();
            new_hothouse.modify()->title = edit->text().toUTF8();
            new_crop.modify()->schedules = db_session_.addNew<models::schedules>();
            try
            {
                constexpr int table_index = 1;
                Wt::WTable& crops_table(dynamic_cast<Wt::WTable&>(*crops_->widget(table_index)));
                add_hothouse_row(crops_table, crops_table.rowCount(), *new_crop);
            }
            catch (const std::bad_cast& error)
            {
                log("error") << error.what();
            }
        }
        root()->removeChild(dialog);
    });

    dialog->show();
}

void application::show_dialog_add_crop()
{
    auto dialog = root()->addNew<Wt::WDialog>(u8"Добавить новую культуру");

    Wt::WLabel* label = dialog->contents()->addNew<Wt::WLabel>(u8"Название");
    Wt::WLineEdit* edit = dialog->contents()->addNew<Wt::WLineEdit>();
    label->setBuddy(edit);

    dialog->contents()->addStyleClass("form-group");

    auto validator = std::make_shared<Wt::WRegExpValidator>(u8"[A-Za-z0-9\x0400-\x04ff]{0,30}");
    validator->setMandatory(true);
    edit->setValidator(validator);

    Wt::WPushButton* ok = dialog->footer()->addNew<Wt::WPushButton>(u8"принять");
    ok->setDefault(true);
    if (wApp->environment().ajax())
    {
        ok->disable();
    }

    Wt::WPushButton* cancel = dialog->footer()->addNew<Wt::WPushButton>(u8"отмена");
    dialog->rejectWhenEscapePressed();
    edit->keyWentUp().connect(
        [ok, edit]
    {
        ok->setDisabled(edit->validate() != Wt::ValidationState::Valid);
    });

    ok->clicked().connect(
        [edit, dialog]
    {
        if (edit->validate() == Wt::ValidationState::Valid)
        {
            dialog->accept();
        }
    });

    cancel->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->finished().connect(
        [this, dialog, edit]
    {
        if (dialog->result() == Wt::DialogCode::Accepted)
        {
            Wt::Dbo::Transaction transaction(db_session_);
            auto new_crop = db_session_.addNew<models::crop>();
            new_crop.modify()->title = edit->text().toUTF8();
            new_crop.modify()->schedules = db_session_.addNew<models::schedules>();
            try
            {
                constexpr int table_index = 1;
                Wt::WTable& crops_table(dynamic_cast<Wt::WTable&>(*crops_->widget(table_index)));
                add_crop_row(crops_table, crops_table.rowCount(), *new_crop);
            }
            catch (const std::bad_cast& error)
            {
                log("error") << error.what();
            }
        }
        root()->removeChild(dialog);  
    });

    dialog->show();
}

void application::show_dialog_change_crop(Wt::WText* crop_title)
{
    auto dialog = root()->addNew<Wt::WDialog>(u8"Изменить культуру");

    auto* label = dialog->contents()->addNew<Wt::WLabel>(u8"Новое название");
    auto* edit = dialog->contents()->addNew<Wt::WLineEdit>();
    label->setBuddy(edit);

    dialog->contents()->addStyleClass("form-group");

    auto validator = std::make_shared<Wt::WRegExpValidator>(u8"[A-Za-z0-9\x0400-\x04ff]{0,30}");
    validator->setMandatory(true);
    edit->setValidator(validator);

    Wt::WPushButton* ok = dialog->footer()->addNew<Wt::WPushButton>(u8"изменить");
    ok->setDefault(true);
    if (wApp->environment().ajax())
    {
        ok->disable();
    }

    Wt::WPushButton* cancel = dialog->footer()->addNew<Wt::WPushButton>(u8"отмена");
    dialog->rejectWhenEscapePressed();
    edit->keyWentUp().connect(
        [ok, edit]
    {
        ok->setDisabled(edit->validate() != Wt::ValidationState::Valid);
    });

    ok->clicked().connect(
        [edit, dialog]
    {
        if (edit->validate() == Wt::ValidationState::Valid)
        {
            dialog->accept();
        }
    });

    cancel->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->finished().connect(
        [this, dialog, edit, crop_title]
    {
        if (dialog->result() == Wt::DialogCode::Accepted)
        {
            Wt::Dbo::Transaction transaction(db_session_);
            Wt::Dbo::ptr<models::crop> crop = db_session_.find<models::crop>().where("title = ?").bind(crop_title->text());
            crop.modify()->title = edit->text().toUTF8();
            crop_title->setText(edit->text());
        }
        root()->removeChild(dialog);
    });

    dialog->show();
}

void application::handle_delete_crop(const Wt::WTableRow* row, const Wt::WString& crop_title)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::crop> crop = db_session_.find<models::crop>().where("title = ?").bind(crop_title);
    crop.remove();
    row->table()->removeRow(row->rowNum());
}

void application::handle_auth()
{
    if (db_session_.login().loggedIn())
    {
        Wt::Dbo::Transaction transaction(db_session_);
        const Wt::Auth::User& u = db_session_.login().user();
        Wt::Dbo::ptr<agromaster::models::AuthInfo> auth_info = db_session_.users().find(u);
        Wt::Dbo::ptr<models::user_account> user_acc = auth_info->user();
        if (!user_acc) {
            user_acc = db_session_.add(std::make_unique<models::user_account>(models::user_account::role::visitor));
            auth_info.modify()->setUser(user_acc);
        }
        user_role_ = user_acc->role;

        Wt::WString login_name = u.identity(Wt::Auth::Identity::LoginName);
        set_navigation_bar(login_name);
        main_stack_ = root()->addNew<Wt::WStackedWidget>();
        main_stack_->setContentAlignment(Wt::AlignmentFlag::Center);
        set_hothouses_table();
        set_crop_table();
        auth_widget_->hide();
        setInternalPath(internal_path::hothouses);

        log("notice")
            << "User " << u.id()
            << " (" << login_name << ")"
            << " logged in.";
    }
    else
    {
        auth_widget_->show();
        root()->removeWidget(navigation_);
        root()->removeWidget(main_stack_);
        setInternalPath(internal_path::root);
        log("notice") << "User logged out.";
    }
}

} // agromaster