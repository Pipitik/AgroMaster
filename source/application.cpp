#include "application.hpp"

#include <numeric>

#include <Wt/Auth/PasswordService.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WBreak.h>
#include <Wt/WCalendar.h>
#include <Wt/WDateEdit.h>
#include <Wt/WDoubleValidator.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>

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

void application::add_hothouse_row(Wt::WTable& table, int index, const agromaster::models::hothouse& hothouse)
{
    constexpr char style_class[] = "text-center";
    auto hothouse_title = table.elementAt(index, 0)->addNew<Wt::WText>(hothouse.title);
    table.elementAt(index, 0)->setStyleClass(style_class);
    auto crop_title = table.elementAt(index, 1)->addNew<Wt::WText>(hothouse.crop ? hothouse.crop->title : u8"не назначено");
    table.elementAt(index, 1)->setStyleClass(style_class);
    auto yields = table.elementAt(index, 2)->addNew<Wt::WText>(std::to_string(hothouse.yields));
    table.elementAt(index, 2)->setStyleClass(style_class);
    auto spent_fertilizers = table.elementAt(index, 3)->addNew<Wt::WText>(std::to_string(hothouse.spent_fertilizers));
    table.elementAt(index, 3)->setStyleClass(style_class);
    table.elementAt(index, 4)->addNew<Wt::WPushButton>(u8"Работы")->
        clicked().connect(
            [this, hothouse_title]()
    {
        show_dialog_hothouse_works(hothouse_title);
    });
    table.elementAt(index, 4)->setStyleClass(style_class);
    if (user_role_ == agromaster::models::user_account::role::admin)
    {
        table.elementAt(index, 5)->addNew<Wt::WPushButton>(u8"Изменить")->
            clicked().connect(
                [this, hothouse_title, crop_title, yields, spent_fertilizers]()
        {
            show_dialog_change_hothouse(hothouse_title, crop_title, yields, spent_fertilizers);
        });
        table.elementAt(index, 5)->setStyleClass(style_class);
        table.elementAt(index, 6)->addNew<Wt::WPushButton>(u8"Удалить")->
            clicked().connect(
                [this, row = table.rowAt(index), hothouse_title]()
        {
            handle_delete_hothouse(row, hothouse_title->text());
        });
        table.elementAt(index, 6)->setStyleClass(style_class);
    }
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

void application::update_hothouses_table()
{
    main_stack_->removeWidget(hothouses_);
    set_hothouses_table();
}

void application::show_dialog_add_hothouse()
{
    auto dialog = root()->addNew<Wt::WDialog>(u8"Добавить новую теплицу");

    Wt::WLabel* label_hothouse_name = dialog->contents()->addNew<Wt::WLabel>(u8"Название");
    Wt::WLineEdit* edit = dialog->contents()->addNew<Wt::WLineEdit>();
    label_hothouse_name->setBuddy(edit);

    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::collection<Wt::Dbo::ptr<models::crop>> crops = db_session_.find<models::crop>();
    Wt::WLabel* label_crop_name = dialog->contents()->addNew<Wt::WLabel>(u8"Культура");
    Wt::WSelectionBox* selection = dialog->contents()->addNew<Wt::WSelectionBox>();
    selection->addItem(u8"не выбрано");
    for (const Wt::Dbo::ptr<models::crop>& crop : crops)
    {
        selection->addItem(crop->title);
    }
    selection->setCurrentIndex(0);
    label_crop_name->setBuddy(selection);

    dialog->contents()->addStyleClass("form-group");

    auto validator = std::make_shared<Wt::WRegExpValidator>(u8"[A-Za-z0-9\x0400-\x04ff]{0,30}");
    validator->setMandatory(true);
    edit->setValidator(validator);

    Wt::WPushButton* ok = dialog->footer()->addNew<Wt::WPushButton>(u8"принять");
    ok->addStyleClass("btn-success");
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
        [this, dialog, edit, selection]
    {
        if (dialog->result() == Wt::DialogCode::Accepted)
        {
            handle_add_hothouse(edit->text().toUTF8(), selection->currentText().toUTF8());
        }
        root()->removeChild(dialog);
    });

    dialog->show();
}

void application::handle_add_hothouse(const std::string& title, const std::string& crop_title)
{

    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::hothouse> existing_hothouse =
        db_session_.find<models::hothouse>().where("title = ?").bind(title).limit(1);
    if (existing_hothouse)
    {
        auto message_box =
            root()->addChild(std::make_unique<Wt::WMessageBox>(
                u8"Ошибка",
                u8"<p>Теплица с таким названием уже существует!</p>",
                Wt::Icon::Critical,
                Wt::StandardButton::Ok));

        message_box->setModal(true);
        message_box->buttonClicked().connect([this, message_box] { root()->removeChild(message_box); });
        message_box->show();
    }
    else
    {
        auto new_hothouse = db_session_.addNew<models::hothouse>();
        new_hothouse.modify()->title = title;
        new_hothouse.modify()->works = db_session_.addNew<models::works>();
        Wt::Dbo::ptr<models::crop> selected_crop;
        if (crop_title != u8"не выбрано")
        {
            selected_crop = db_session_.find<models::crop>().where("title = ?").bind(crop_title);
        }
        if (selected_crop)
        {
            new_hothouse.modify()->crop = selected_crop;
            selected_crop.modify()->hothouses.insert(new_hothouse);
        }

        try
        {
            constexpr int table_index = 1;
            Wt::WTable& hothouses_table(dynamic_cast<Wt::WTable&>(*hothouses_->widget(table_index)));
            add_hothouse_row(hothouses_table, hothouses_table.rowCount(), *new_hothouse);
            update_crops_table();
        }
        catch (const std::bad_cast& error)
        {
            log("error") << error.what();
        }
    }
}

void application::show_dialog_change_hothouse(
    Wt::WText* current_title,
    Wt::WText* current_crop_title,
    Wt::WText* current_yields,
    Wt::WText* current_spent_fertilizers)
{
    auto dialog = root()->addNew<Wt::WDialog>(u8"Изменить теплицу");

    auto* label_new_hothouse_name = dialog->contents()->addNew<Wt::WLabel>(u8"Новое название");
    auto* edit_hothouse_name = dialog->contents()->addNew<Wt::WLineEdit>();
    edit_hothouse_name->setText(current_title->text());
    label_new_hothouse_name->setBuddy(edit_hothouse_name);

    auto* label_yields = dialog->contents()->addNew<Wt::WLabel>(u8"Количество урожая");
    auto* edit_yields = dialog->contents()->addNew<Wt::WLineEdit>();
    edit_yields->setText(current_yields->text());
    label_new_hothouse_name->setBuddy(edit_yields);

    auto* label_spent_fertilizers = dialog->contents()->addNew<Wt::WLabel>(u8"Количество затраченного удобрения");
    auto* edit_spent_fertilizers = dialog->contents()->addNew<Wt::WLineEdit>();
    edit_spent_fertilizers->setText(current_spent_fertilizers->text());
    label_new_hothouse_name->setBuddy(edit_spent_fertilizers);

    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::collection<Wt::Dbo::ptr<models::crop>> crops = db_session_.find<models::crop>();
    auto* label_crop_name = dialog->contents()->addNew<Wt::WLabel>(u8"Культура");
    auto* selection = dialog->contents()->addNew<Wt::WSelectionBox>();
    selection->addItem(u8"не выбрано");
    std::size_t i = 0;
    selection->setCurrentIndex(i);
    std::string current_crop_title_string = current_crop_title->text().toUTF8();
    for (const Wt::Dbo::ptr<models::crop>& crop : crops)
    {
        selection->addItem(crop->title);
        ++i;
        if (crop->title == current_crop_title_string)
        {
            selection->setCurrentIndex(i);
        }
    }
    label_crop_name->setBuddy(selection);

    dialog->contents()->addStyleClass("form-group");

    auto validator = std::make_shared<Wt::WRegExpValidator>(u8"[A-Za-z0-9\x0400-\x04ff]{0,30}");
    validator->setMandatory(false);
    edit_hothouse_name->setValidator(validator);
    edit_yields->setValidator(std::make_shared<Wt::WDoubleValidator>());
    edit_spent_fertilizers->setValidator(std::make_shared<Wt::WDoubleValidator>());

    Wt::WPushButton* ok = dialog->footer()->addNew<Wt::WPushButton>(u8"изменить");
    ok->setDefault(true);

    Wt::WPushButton* cancel = dialog->footer()->addNew<Wt::WPushButton>(u8"отмена");
    dialog->rejectWhenEscapePressed();
    edit_hothouse_name->keyWentUp().connect(
        [ok, edit_hothouse_name]
    {
        ok->setDisabled(edit_hothouse_name->validate() != Wt::ValidationState::Valid);
    });
    edit_yields->keyWentUp().connect(
        [ok, edit_yields]
    {
        ok->setDisabled(edit_yields->validate() != Wt::ValidationState::Valid);
    });
    edit_spent_fertilizers->keyWentUp().connect(
        [ok, edit_spent_fertilizers]
    {
        ok->setDisabled(edit_spent_fertilizers->validate() != Wt::ValidationState::Valid);
    });
    selection->keyWentUp().connect(
        [ok, selection]
    {
        ok->setDisabled(selection->currentIndex() == -1);
    });

    ok->clicked().connect(
        [dialog, edit_hothouse_name, edit_yields, edit_spent_fertilizers]
    {
        if (edit_hothouse_name->validate() == Wt::ValidationState::Valid &&
            edit_yields->validate() == Wt::ValidationState::Valid &&
            edit_spent_fertilizers->validate() == Wt::ValidationState::Valid)
        {
            dialog->accept();
        }
    });

    cancel->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->finished().connect(
        [this, dialog,
        edit_hothouse_name, edit_yields, edit_spent_fertilizers, selection,
        current_title, current_crop_title, current_yields, current_spent_fertilizers]
    {
        if (dialog->result() == Wt::DialogCode::Accepted)
        {
            handle_change_hothouse(
                edit_hothouse_name->text().toUTF8(),
                selection->currentText().toUTF8(),
                edit_yields->text().toUTF8(),
                edit_spent_fertilizers->text().toUTF8(),
                current_title,
                current_crop_title,
                current_yields,
                current_spent_fertilizers);
        }
        root()->removeChild(dialog);
    });

    dialog->show();
}

void application::handle_change_hothouse(
    const std::string& new_title,
    const std::string& new_crop,
    const std::string& new_yields,
    const std::string& new_spent_fertilizers,
    Wt::WText* current_title,
    Wt::WText* current_crop_title,
    Wt::WText* current_yields,
    Wt::WText* current_spent_fertilizers)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::hothouse> hothouse =
        db_session_.find<models::hothouse>().where("title = ?").bind(current_title->text()).limit(1);

    if (!new_title.empty() && hothouse->title != new_title)
    {
        hothouse.modify()->title = new_title;
        current_title->setText(new_title);
    }

    double new_yields_double = std::stod(new_yields);
    if (hothouse->yields != new_yields_double)
    {
        hothouse.modify()->yields = new_yields_double;
        current_yields->setText(new_yields);
    }

    double new_spent_fertilizers_double = std::stod(new_spent_fertilizers);
    if (hothouse->spent_fertilizers != new_spent_fertilizers_double)
    {
        hothouse.modify()->spent_fertilizers = new_spent_fertilizers_double;
        current_spent_fertilizers->setText(new_spent_fertilizers);
    }

    if (!hothouse->crop && new_crop != u8"не выбрано")
    {
        Wt::Dbo::ptr<models::crop> selected_crop =
            db_session_.find<models::crop>().where("title = ?").bind(new_crop);
        hothouse.modify()->crop = selected_crop;
        selected_crop.modify()->hothouses.insert(hothouse);
        current_crop_title->setText(new_crop);
    }
    else if (hothouse->crop && new_crop != hothouse->crop->title)
    {
        if (new_crop == u8"не выбрано")
        {
            hothouse->crop.modify()->hothouses.erase(hothouse);
            hothouse.modify()->crop = nullptr;
            current_crop_title->setText(u8"не назначено");
        }
        else
        {
            Wt::Dbo::ptr<models::crop> selected_crop =
                db_session_.find<models::crop>().where("title = ?").bind(new_crop);
            hothouse->crop.modify()->hothouses.erase(hothouse);
            hothouse.modify()->crop = selected_crop;
            selected_crop.modify()->hothouses.insert(hothouse);
            current_crop_title->setText(new_crop);
        }
    }
    update_crops_table();
}

void application::show_dialog_hothouse_works(Wt::WText* title)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::hothouse> hothouse =
        db_session_.find<models::hothouse>().where("title = ?").bind(title->text()).limit(1);

    Wt::Dbo::ptr<models::works> works = hothouse->works.lock();
    assert(works);

    auto dialog = root()->addNew<Wt::WDialog>(u8"Выполненные работы");
    dialog->setScrollVisibilityEnabled(true);
    dialog->setMaximumSize("90%", "90%");
    
    auto* sowing_date_label = dialog->contents()->addNew<Wt::WLabel>(u8"Посадка");
    auto* sowing_date_edit = dialog->contents()->addNew<Wt::WDateEdit>();
    sowing_date_edit->setDate(works->sowing_work);
    if (user_role_ == models::user_account::role::admin)
    {
        sowing_date_edit->setSelectable(true);
    }
    else
    {
        sowing_date_edit->setSelectable(false);
    }
    sowing_date_label->setBuddy(sowing_date_edit);

    dialog->contents()->addNew<Wt::WBreak>();

    auto* harvest_date_label = dialog->contents()->addNew<Wt::WLabel>(u8"Сбор урожая");
    auto* harvest_date_edit = dialog->contents()->addNew<Wt::WDateEdit>();
    harvest_date_edit->setDate(works->harvest_work);
    if (user_role_ == models::user_account::role::admin)
    {
        harvest_date_edit->setSelectable(true);
    }
    else
    {
        harvest_date_edit->setSelectable(false);
    }
    harvest_date_label->setBuddy(harvest_date_edit);

    dialog->contents()->addNew<Wt::WBreak>();

    std::set<Wt::WDate> dates;
    auto* label_calendar_fertilizer = dialog->contents()->addNew<Wt::WLabel>(u8"Удобрения");
    auto* calendar_fertilizer = dialog->contents()->addNew<Wt::WCalendar>();
    calendar_fertilizer->setSelectionMode(Wt::SelectionMode::Extended);
    for (const auto& pdate : works->fertilizer_works)
    {
        dates.insert(pdate->date);
    }
    calendar_fertilizer->select(dates);

    if (user_role_ == models::user_account::role::admin)
    {
        calendar_fertilizer->setSelectable(true);
    }
    else
    {
        calendar_fertilizer->setSelectable(false);
    }

    dialog->contents()->addNew<Wt::WBreak>();

    dates.clear();
    auto* label_calendar_watering = dialog->contents()->addNew<Wt::WLabel>(u8"Полив");
    auto* calendar_watering = dialog->contents()->addNew<Wt::WCalendar>();
    calendar_watering->setSelectionMode(Wt::SelectionMode::Extended);
    for (const auto& pdate : works->watering_works)
    {
        dates.insert(pdate->date);
    }
    calendar_watering->select(dates);

    if (user_role_ == models::user_account::role::admin)
    {
        calendar_watering->setSelectable(true);
    }
    else
    {
        calendar_watering->setSelectable(false);
    }

    dialog->contents()->addStyleClass("form-group");

    if (user_role_ == models::user_account::role::admin)
    {
        Wt::WPushButton* ok = dialog->footer()->addNew<Wt::WPushButton>(u8"изменить");
        ok->setDefault(true);
        ok->addStyleClass("btn-success");
        ok->clicked().connect(
            [dialog]
        {
            dialog->accept();
        });
    }

    Wt::WPushButton* quit = dialog->footer()->addNew<Wt::WPushButton>(u8"выйти");
    dialog->rejectWhenEscapePressed();
    quit->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->finished().connect(
        [this, dialog, title, sowing_date_edit, harvest_date_edit, calendar_fertilizer, calendar_watering]
    {
        if (dialog->result() == Wt::DialogCode::Accepted)
        {
            handle_change_hothouse_works(
                title->text().toUTF8(),
                sowing_date_edit->date(),
                harvest_date_edit->date(),
                calendar_fertilizer->selection(),
                calendar_watering->selection());
        }
        root()->removeChild(dialog);
    });

    dialog->show();
}

void application::handle_change_hothouse_works(
    const std::string& title,
    const Wt::WDate& sowing_date,
    const Wt::WDate& harvest_date,
    const std::set<Wt::WDate>& fertilizer_dates,
    const std::set<Wt::WDate>& watering_dates)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::hothouse> hothouse =
        db_session_.find<models::hothouse>().where("title = ?").bind(title).limit(1);
    Wt::Dbo::ptr<models::works> works = hothouse->works.lock();
    assert(works);

    if (sowing_date != works->sowing_work)
    {
        works.modify()->sowing_work = sowing_date;
    }

    if (harvest_date != works->harvest_work)
    {
        works.modify()->harvest_work = harvest_date;
    }

    works.modify()->fertilizer_works.clear();
    for (const Wt::WDate& date : fertilizer_dates)
    {
        works.modify()->fertilizer_works.insert(db_session_.addNew<models::fertilizer_works>(date));
    }

    works.modify()->watering_works.clear();
    for (const Wt::WDate& date : watering_dates)
    {
        works.modify()->watering_works.insert(db_session_.addNew<models::watering_works>(date));
    }
}

void application::handle_delete_hothouse(const Wt::WTableRow* row, const Wt::WString& hothouse_title)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::hothouse> hothouse =
        db_session_.find<models::hothouse>().where("title = ?").bind(hothouse_title).limit(1);
    hothouse.remove();
    row->table()->removeRow(row->rowNum());
    update_crops_table();
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
    table.elementAt(index, 4)->addNew<Wt::WPushButton>(u8"Графики")->
        clicked().connect(
            [this, crop_title]()
    {
        show_dialog_crop_schedules(crop_title);
    });
    table.elementAt(index, 4)->setStyleClass(style_class);
    if (user_role_ == agromaster::models::user_account::role::admin)
    {
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

void application::set_crops_table()
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
    crops_table->elementAt(i, 4)->addNew<Wt::WText>("");
    if (user_role_ == agromaster::models::user_account::role::admin)
    {
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

void application::update_crops_table()
{
    main_stack_->removeWidget(crops_);
    set_crops_table();
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
    ok->addStyleClass("btn-success");
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
            handle_add_crop(edit->text().toUTF8());
        }
        root()->removeChild(dialog);
    });

    dialog->show();
}

void application::handle_add_crop(const std::string& title)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::crop> existing_crop =
        db_session_.find<models::crop>().where("title = ?").bind(title).limit(1);
    if (existing_crop)
    {
        auto message_box =
            root()->addChild(std::make_unique<Wt::WMessageBox>(
                u8"Ошибка",
                u8"<p>Культура с таким названием уже существует!</p>",
                Wt::Icon::Critical,
                Wt::StandardButton::Ok));

        message_box->setModal(true);
        message_box->buttonClicked().connect([this ,message_box] { root()->removeChild(message_box); });
        message_box->show();
    }
    else
    {
        auto new_crop = db_session_.addNew<models::crop>();
        new_crop.modify()->title = title;
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
}

void application::show_dialog_change_crop(Wt::WText* title)
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
    ok->addStyleClass("btn-success");
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
        [this, dialog, edit, title]
    {
        if (dialog->result() == Wt::DialogCode::Accepted)
        {
            Wt::Dbo::Transaction transaction(db_session_);
            Wt::Dbo::ptr<models::crop> crop =
                db_session_.find<models::crop>().where("title = ?").bind(title->text()).limit(1);
            crop.modify()->title = edit->text().toUTF8();
            title->setText(edit->text());
            update_hothouses_table();
        }
        root()->removeChild(dialog);
    });

    dialog->show();
}

void application::show_dialog_crop_schedules(Wt::WText* title)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::crop> crop =
        db_session_.find<models::crop>().where("title = ?").bind(title->text()).limit(1);

    Wt::Dbo::ptr<models::schedules> schedules = crop->schedules.lock();
    assert(schedules);

    auto dialog = root()->addNew<Wt::WDialog>(u8"Графики");
    dialog->setScrollVisibilityEnabled(true);
    dialog->setMaximumSize("90%", "90%");

    auto* sowing_date_label = dialog->contents()->addNew<Wt::WLabel>(u8"График посадки");
    auto* sowing_date_edit = dialog->contents()->addNew<Wt::WDateEdit>();
    sowing_date_edit->setDate(schedules->sowing_schedule);
    if (user_role_ == models::user_account::role::admin)
    {
        sowing_date_edit->setSelectable(true);
    }
    else
    {
        sowing_date_edit->setSelectable(false);
    }
    sowing_date_label->setBuddy(sowing_date_edit);

    dialog->contents()->addNew<Wt::WBreak>();

    auto* harvest_date_label = dialog->contents()->addNew<Wt::WLabel>(u8"График сбора урожая");
    auto* harvest_date_edit = dialog->contents()->addNew<Wt::WDateEdit>();
    harvest_date_edit->setDate(schedules->harvest_schedule);
    if (user_role_ == models::user_account::role::admin)
    {
        harvest_date_edit->setSelectable(true);
    }
    else
    {
        harvest_date_edit->setSelectable(false);
    }
    harvest_date_label->setBuddy(harvest_date_edit);

    dialog->contents()->addNew<Wt::WBreak>();

    std::set<Wt::WDate> dates;
    auto* label_calendar_fertilizer = dialog->contents()->addNew<Wt::WLabel>(u8"График удобрений");
    auto* calendar_fertilizer = dialog->contents()->addNew<Wt::WCalendar>();
    calendar_fertilizer->setSelectionMode(Wt::SelectionMode::Extended);
    for (const auto& pdate : schedules->fertilizer_schedules)
    {
        dates.insert(pdate->date);
    }
    calendar_fertilizer->select(dates);

    if (user_role_ == models::user_account::role::admin)
    {
        calendar_fertilizer->setSelectable(true);
    }
    else
    {
        calendar_fertilizer->setSelectable(false);
    }

    dialog->contents()->addNew<Wt::WBreak>();
    
    dates.clear();
    auto* label_calendar_watering = dialog->contents()->addNew<Wt::WLabel>(u8"График полива");
    auto* calendar_watering = dialog->contents()->addNew<Wt::WCalendar>();
    calendar_watering->setSelectionMode(Wt::SelectionMode::Extended);
    for (const auto& pdate : schedules->watering_schedules)
    {
        dates.insert(pdate->date);
    }
    calendar_watering->select(dates);

    if (user_role_ == models::user_account::role::admin)
    {
        calendar_watering->setSelectable(true);
    }
    else
    {
        calendar_watering->setSelectable(false);
    }

    dialog->contents()->addStyleClass("form-group");

    if (user_role_ == models::user_account::role::admin)
    {
        Wt::WPushButton* ok = dialog->footer()->addNew<Wt::WPushButton>(u8"изменить");
        ok->setDefault(true);
        ok->addStyleClass("btn-success");
        ok->clicked().connect(
            [dialog]
        {
            dialog->accept();
        });
    }

    Wt::WPushButton* quit = dialog->footer()->addNew<Wt::WPushButton>(u8"выйти");
    dialog->rejectWhenEscapePressed();
    quit->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->finished().connect(
        [this, dialog, title, sowing_date_edit, harvest_date_edit, calendar_fertilizer, calendar_watering]
    {
        if (dialog->result() == Wt::DialogCode::Accepted)
        {
            handle_change_crop_schedules(
                title->text().toUTF8(),
                sowing_date_edit->date(),
                harvest_date_edit->date(),
                calendar_fertilizer->selection(),
                calendar_watering->selection());
        }
        root()->removeChild(dialog);
    });

    dialog->show();
}

void application::handle_change_crop_schedules(
    const std::string& title,
    const Wt::WDate& sowing_date,
    const Wt::WDate& harvest_date,
    const std::set<Wt::WDate>& fertilizer_dates,
    const std::set<Wt::WDate>& watering_dates)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::crop> crop =
        db_session_.find<models::crop>().where("title = ?").bind(title).limit(1);
    Wt::Dbo::ptr<models::schedules> schedules = crop->schedules.lock();
    assert(schedules);

    if (sowing_date != schedules->sowing_schedule)
    {
        schedules.modify()->sowing_schedule = sowing_date;
    }

    if (harvest_date != schedules->harvest_schedule)
    {
        schedules.modify()->harvest_schedule = harvest_date;
    }

    schedules.modify()->fertilizer_schedules.clear();
    for (const Wt::WDate& date : fertilizer_dates)
    {
        schedules.modify()->fertilizer_schedules.insert(db_session_.addNew<models::fertilizer_schedules>(date));
    }

    schedules.modify()->watering_schedules.clear();
    for (const Wt::WDate& date : watering_dates)
    {
        schedules.modify()->watering_schedules.insert(db_session_.addNew<models::watering_schedules>(date));
    }
}

void application::handle_delete_crop(const Wt::WTableRow* row, const Wt::WString& crop_title)
{
    Wt::Dbo::Transaction transaction(db_session_);
    Wt::Dbo::ptr<models::crop> crop =
        db_session_.find<models::crop>().where("title = ?").bind(crop_title).limit(1);
    crop.remove();
    row->table()->removeRow(row->rowNum());
    update_hothouses_table();
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
        set_crops_table();
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