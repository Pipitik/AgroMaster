#include "application.hpp"

void try_create_database(Wt::Dbo::SqlConnectionPool& pool)
{
    using namespace agromaster;

    models::session session(pool);

    try
    {
        session.createTables();

        Wt::Dbo::Transaction transaction(session);
        Wt::Auth::User admin = session.users().registerNew();
        admin.addIdentity(Wt::Auth::Identity::LoginName, "admin");
        session.users().setPassword(admin, models::session::password_auth().verifier()->hashPassword("admin"));

        Wt::Dbo::ptr<agromaster::models::AuthInfo> auth_info = session.users().find(admin);
        Wt::Dbo::ptr<models::user_account> user_acc = auth_info->user();
        if (!user_acc) {
            user_acc = session.add(std::make_unique<models::user_account>(models::user_account::role::admin));
            auth_info.modify()->setUser(user_acc);
        }

        std::clog << "Created database." << std::endl;
    }
    catch (Wt::Dbo::Exception& error)
    {
        std::clog
            << error.what() << '\n'
            << "Using existing database" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    try {
        Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

        agromaster::models::session::configure_auth();

        auto connection = std::make_unique<Wt::Dbo::backend::Postgres>(
            "host=localhost password=example dbname=agronomy user=postgres");

        auto connection_pool = std::make_unique<Wt::Dbo::FixedSqlConnectionPool>(std::move(connection), 10);
        try_create_database(*connection_pool);

        server.addEntryPoint(Wt::EntryPointType::Application,
            [&connection_pool](const Wt::WEnvironment& env)
        {
            return std::make_unique<agromaster::application>(env, *connection_pool);
        });

        server.run();
    }
    catch (const Wt::WServer::Exception& error)
    {
        std::cerr << error.what() << std::endl;
    }
    catch (const std::exception& error)
    {
        std::cerr << "exception: " << error.what() << std::endl;
    }
}