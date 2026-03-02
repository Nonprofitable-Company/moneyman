#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include "db/database.h"

#include <QCoreApplication>
#include <QTemporaryDir>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int result = Catch::Session().run(argc, argv);
    return result;
}

TEST_CASE("Database open and schema creation", "[db]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    Database db;
    QString path = tmpDir.path() + "/test.db";
    REQUIRE(db.open(path));
    REQUIRE(db.isOpen());

    db.close();
    REQUIRE_FALSE(db.isOpen());
}

TEST_CASE("Account CRUD", "[db]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    Database db;
    REQUIRE(db.open(tmpDir.path() + "/test.db"));

    SECTION("create and retrieve accounts") {
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        REQUIRE(db.createAccount(2000, "Accounts Payable", "liability"));

        auto accounts = db.allAccounts();
        REQUIRE(accounts.size() == 2);
        REQUIRE(accounts[0].code == 1000);
        REQUIRE(accounts[0].name == "Cash");
        REQUIRE(accounts[0].type == "asset");
        REQUIRE(accounts[1].code == 2000);
    }

    SECTION("lookup by code") {
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        auto acct = db.accountByCode(1000);
        REQUIRE(acct.name == "Cash");
    }

    SECTION("reject duplicate code") {
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        REQUIRE_FALSE(db.createAccount(1000, "Other Cash", "asset"));
    }
}

TEST_CASE("Journal entry posting", "[db]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    Database db;
    REQUIRE(db.open(tmpDir.path() + "/test.db"));

    REQUIRE(db.createAccount(1000, "Cash", "asset"));
    REQUIRE(db.createAccount(4000, "Sales", "revenue"));

    auto cash = db.accountByCode(1000);
    auto sales = db.accountByCode(4000);

    SECTION("post balanced entry updates balances") {
        std::vector<JournalLineRow> lines = {
            {0, 0, cash.id, 50000, 0},
            {0, 0, sales.id, 0, 50000},
        };
        REQUIRE(db.postJournalEntry("2026-03-01", "Sale to customer", lines));

        auto updatedCash = db.accountByCode(1000);
        auto updatedSales = db.accountByCode(4000);
        REQUIRE(updatedCash.balanceCents == 50000);
        REQUIRE(updatedSales.balanceCents == 50000);
    }

    SECTION("reject unbalanced entry") {
        std::vector<JournalLineRow> lines = {
            {0, 0, cash.id, 50000, 0},
            {0, 0, sales.id, 0, 30000},
        };
        REQUIRE_FALSE(db.postJournalEntry("2026-03-01", "Bad entry", lines));
    }

    SECTION("reject single line entry") {
        std::vector<JournalLineRow> lines = {
            {0, 0, cash.id, 50000, 0},
        };
        REQUIRE_FALSE(db.postJournalEntry("2026-03-01", "Bad entry", lines));
    }
}
