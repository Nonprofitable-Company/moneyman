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

TEST_CASE("Fiscal period closing", "[db][fiscal]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    Database db;
    REQUIRE(db.open(tmpDir.path() + "/test.db"));

    // Set up chart of accounts
    REQUIRE(db.createAccount(1000, "Cash", "asset"));
    REQUIRE(db.createAccount(2000, "Accounts Payable", "liability"));
    REQUIRE(db.createAccount(4000, "Sales Revenue", "revenue"));
    REQUIRE(db.createAccount(4100, "Service Revenue", "revenue"));
    REQUIRE(db.createAccount(5000, "Rent Expense", "expense"));
    REQUIRE(db.createAccount(5100, "Supplies Expense", "expense"));

    auto cash = db.accountByCode(1000);
    auto sales = db.accountByCode(4000);
    auto service = db.accountByCode(4100);
    auto rent = db.accountByCode(5000);
    auto supplies = db.accountByCode(5100);

    // Post some transactions
    std::vector<JournalLineRow> entry1 = {
        {0, 0, cash.id, 100000, 0},       // Debit Cash 1000.00
        {0, 0, sales.id, 0, 100000},      // Credit Sales 1000.00
    };
    REQUIRE(db.postJournalEntry("2026-01-15", "Cash sale", entry1));

    std::vector<JournalLineRow> entry2 = {
        {0, 0, cash.id, 50000, 0},        // Debit Cash 500.00
        {0, 0, service.id, 0, 50000},     // Credit Service 500.00
    };
    REQUIRE(db.postJournalEntry("2026-02-10", "Service income", entry2));

    std::vector<JournalLineRow> entry3 = {
        {0, 0, rent.id, 30000, 0},        // Debit Rent 300.00
        {0, 0, cash.id, 0, 30000},        // Credit Cash 300.00
    };
    REQUIRE(db.postJournalEntry("2026-03-01", "Rent payment", entry3));

    std::vector<JournalLineRow> entry4 = {
        {0, 0, supplies.id, 10000, 0},    // Debit Supplies 100.00
        {0, 0, cash.id, 0, 10000},        // Credit Cash 100.00
    };
    REQUIRE(db.postJournalEntry("2026-03-15", "Office supplies", entry4));

    SECTION("ensureRetainedEarningsAccount auto-creates account 3100") {
        // Should not exist yet
        auto before = db.accountByCode(3100);
        REQUIRE(before.id == 0);

        int64_t id = db.ensureRetainedEarningsAccount();
        REQUIRE(id > 0);

        auto after = db.accountByCode(3100);
        REQUIRE(after.id == id);
        REQUIRE(after.name == "Retained Earnings");
        REQUIRE(after.type == "equity");

        // Calling again returns the same id
        int64_t id2 = db.ensureRetainedEarningsAccount();
        REQUIRE(id2 == id);
    }

    SECTION("closePeriod zeros revenue and expense accounts") {
        // Verify pre-close balances
        REQUIRE(db.accountByCode(4000).balanceCents == 100000);
        REQUIRE(db.accountByCode(4100).balanceCents == 50000);
        REQUIRE(db.accountByCode(5000).balanceCents == 30000);
        REQUIRE(db.accountByCode(5100).balanceCents == 10000);

        REQUIRE(db.closePeriod("2026-01-01", "2026-12-31"));

        // Revenue and expense accounts should be zeroed
        REQUIRE(db.accountByCode(4000).balanceCents == 0);
        REQUIRE(db.accountByCode(4100).balanceCents == 0);
        REQUIRE(db.accountByCode(5000).balanceCents == 0);
        REQUIRE(db.accountByCode(5100).balanceCents == 0);

        // Retained Earnings should have net income: (1000+500) - (300+100) = 1100.00
        auto re = db.accountByCode(3100);
        REQUIRE(re.id > 0);
        REQUIRE(re.balanceCents == 110000);
    }

    SECTION("closePeriod creates closing journal entry") {
        REQUIRE(db.closePeriod("2026-01-01", "2026-12-31"));

        auto entries = db.allJournalEntries();
        // 4 original + 1 closing = 5
        REQUIRE(entries.size() == 5);

        auto &closing = entries.back();
        REQUIRE(closing.posted == true);
        REQUIRE(closing.description.contains("Closing entry"));
        // 4 temp accounts + 1 retained earnings = 5 lines
        REQUIRE(closing.lines.size() == 5);
    }

    SECTION("closePeriod records fiscal period") {
        REQUIRE(db.closePeriod("2026-01-01", "2026-06-30"));

        auto periods = db.allFiscalPeriods();
        REQUIRE(periods.size() == 1);
        REQUIRE(periods[0].startDate == "2026-01-01");
        REQUIRE(periods[0].endDate == "2026-06-30");
        REQUIRE_FALSE(periods[0].closedAt.isEmpty());
    }

    SECTION("prevents overlapping periods") {
        REQUIRE(db.closePeriod("2026-01-01", "2026-06-30"));
        REQUIRE_FALSE(db.closePeriod("2026-03-01", "2026-12-31"));
    }

    SECTION("prevents posting to closed periods") {
        REQUIRE(db.closePeriod("2026-01-01", "2026-06-30"));

        std::vector<JournalLineRow> lines = {
            {0, 0, cash.id, 5000, 0},
            {0, 0, sales.id, 0, 5000},
        };
        // Date within closed period should fail
        REQUIRE_FALSE(db.postJournalEntry("2026-04-01", "Should fail", lines));

        // Date outside closed period should succeed
        REQUIRE(db.postJournalEntry("2026-07-01", "Should work", lines));
    }

    SECTION("closePeriod fails with no revenue or expense balances") {
        // Close once to zero everything
        REQUIRE(db.closePeriod("2026-01-01", "2026-06-30"));

        // Try closing again for a different period — no balances left
        REQUIRE_FALSE(db.closePeriod("2026-07-01", "2026-12-31"));
    }
}
