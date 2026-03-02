#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include "db/database.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "utils/csv_import.h"

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

    SECTION("create account with currency") {
        REQUIRE(db.createAccount(1000, "Cash", "asset", "USD"));
        REQUIRE(db.createAccount(1100, "Euro Cash", "asset", "EUR"));

        auto usd = db.accountByCode(1000);
        REQUIRE(usd.currency == "USD");

        auto eur = db.accountByCode(1100);
        REQUIRE(eur.currency == "EUR");
    }

    SECTION("default currency is USD") {
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        auto acct = db.accountByCode(1000);
        REQUIRE(acct.currency == "USD");
    }

    SECTION("update account name and type") {
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        auto acct = db.accountByCode(1000);
        REQUIRE(acct.id > 0);

        REQUIRE(db.updateAccount(acct.id, "Petty Cash", "asset"));
        auto updated = db.accountByCode(1000);
        REQUIRE(updated.name == "Petty Cash");
        REQUIRE(updated.type == "asset");
        REQUIRE(updated.code == 1000); // code unchanged
    }

    SECTION("update account type") {
        REQUIRE(db.createAccount(5000, "Misc", "expense"));
        auto acct = db.accountByCode(5000);

        REQUIRE(db.updateAccount(acct.id, "Misc Revenue", "revenue"));
        auto updated = db.accountByCode(5000);
        REQUIRE(updated.name == "Misc Revenue");
        REQUIRE(updated.type == "revenue");
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

TEST_CASE("Void journal entry", "[db][void]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    Database db;
    REQUIRE(db.open(tmpDir.path() + "/test.db"));

    REQUIRE(db.createAccount(1000, "Cash", "asset"));
    REQUIRE(db.createAccount(4000, "Sales Revenue", "revenue"));

    auto cash = db.accountByCode(1000);
    auto sales = db.accountByCode(4000);

    std::vector<JournalLineRow> lines = {
        {0, 0, cash.id, 75000, 0},
        {0, 0, sales.id, 0, 75000},
    };
    REQUIRE(db.postJournalEntry("2026-05-01", "Cash sale", lines));

    // Verify initial balances
    REQUIRE(db.accountByCode(1000).balanceCents == 75000);
    REQUIRE(db.accountByCode(4000).balanceCents == 75000);

    SECTION("voiding zeros out balance impact") {
        auto entries = db.allJournalEntries();
        REQUIRE(entries.size() == 1);
        int64_t entryId = entries[0].id;

        REQUIRE(db.voidJournalEntry(entryId));

        // Balances should be back to zero
        REQUIRE(db.accountByCode(1000).balanceCents == 0);
        REQUIRE(db.accountByCode(4000).balanceCents == 0);

        // Original entry marked as voided
        auto updatedEntry = db.journalEntryById(entryId);
        REQUIRE(updatedEntry.description.startsWith("[VOIDED]"));

        // Reversing entry created
        auto allEntries = db.allJournalEntries();
        REQUIRE(allEntries.size() == 2);
        REQUIRE(allEntries[1].description.contains("Reversal"));

        // Reversing entry has swapped debits/credits
        REQUIRE(allEntries[1].lines.size() == 2);
        // Cash line: original was debit 750, reversal should be credit 750
        bool foundCashReversal = false;
        for (const auto &line : allEntries[1].lines) {
            if (line.accountId == cash.id) {
                REQUIRE(line.debitCents == 0);
                REQUIRE(line.creditCents == 75000);
                foundCashReversal = true;
            }
        }
        REQUIRE(foundCashReversal);
    }

    SECTION("cannot void already voided entry") {
        auto entries = db.allJournalEntries();
        int64_t entryId = entries[0].id;

        REQUIRE(db.voidJournalEntry(entryId));
        REQUIRE_FALSE(db.voidJournalEntry(entryId));
    }

    SECTION("cannot void nonexistent entry") {
        REQUIRE_FALSE(db.voidJournalEntry(99999));
    }
}

TEST_CASE("Journal templates", "[db][template]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    Database db;
    REQUIRE(db.open(tmpDir.path() + "/test.db"));

    REQUIRE(db.createAccount(1000, "Cash", "asset"));
    REQUIRE(db.createAccount(5000, "Rent Expense", "expense"));

    auto cash = db.accountByCode(1000);
    auto rent = db.accountByCode(5000);

    std::vector<JournalLineRow> lines = {
        {0, 0, cash.id, 0, 150000},
        {0, 0, rent.id, 150000, 0},
    };

    SECTION("save and retrieve template") {
        REQUIRE(db.saveTemplate("Monthly Rent", "Rent payment", lines));

        auto templates = db.allTemplates();
        REQUIRE(templates.size() == 1);
        REQUIRE(templates[0].name == "Monthly Rent");
        REQUIRE(templates[0].description == "Rent payment");
        REQUIRE(templates[0].lines.size() == 2);
    }

    SECTION("retrieve template by id") {
        REQUIRE(db.saveTemplate("Monthly Rent", "Rent payment", lines));
        auto all = db.allTemplates();
        auto tpl = db.templateById(all[0].id);
        REQUIRE(tpl.name == "Monthly Rent");
        REQUIRE(tpl.lines.size() == 2);
    }

    SECTION("delete template") {
        REQUIRE(db.saveTemplate("Monthly Rent", "Rent payment", lines));
        auto all = db.allTemplates();
        REQUIRE(db.deleteTemplate(all[0].id));
        REQUIRE(db.allTemplates().empty());
    }

    SECTION("reject duplicate template name") {
        REQUIRE(db.saveTemplate("Monthly Rent", "Rent payment", lines));
        REQUIRE_FALSE(db.saveTemplate("Monthly Rent", "Different desc", lines));
    }

    SECTION("reject template with less than 2 lines") {
        std::vector<JournalLineRow> singleLine = {
            {0, 0, cash.id, 0, 150000},
        };
        REQUIRE_FALSE(db.saveTemplate("Bad Template", "test", singleLine));
    }
}

TEST_CASE("Audit log", "[db][audit]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    Database db;
    REQUIRE(db.open(tmpDir.path() + "/test.db"));

    SECTION("records account creation") {
        REQUIRE(db.allAuditLog().empty());
        REQUIRE(db.createAccount(1000, "Cash", "asset"));

        auto log = db.allAuditLog();
        REQUIRE(log.size() == 1);
        REQUIRE(log[0].action == "CREATE_ACCOUNT");
        REQUIRE(log[0].details.contains("Cash"));
        REQUIRE_FALSE(log[0].timestamp.isEmpty());
    }

    SECTION("records journal entry posting") {
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        REQUIRE(db.createAccount(4000, "Sales", "revenue"));
        auto cash = db.accountByCode(1000);
        auto sales = db.accountByCode(4000);

        std::vector<JournalLineRow> lines = {
            {0, 0, cash.id, 10000, 0},
            {0, 0, sales.id, 0, 10000},
        };
        REQUIRE(db.postJournalEntry("2026-01-01", "Test sale", lines));

        auto log = db.allAuditLog();
        // 2 account creations + 1 journal post = 3
        REQUIRE(log.size() == 3);
        // Most recent first
        REQUIRE(log[0].action == "POST_JOURNAL_ENTRY");
        REQUIRE(log[0].details.contains("Test sale"));
    }

    SECTION("records voiding") {
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        REQUIRE(db.createAccount(4000, "Sales", "revenue"));
        auto cash = db.accountByCode(1000);
        auto sales = db.accountByCode(4000);

        std::vector<JournalLineRow> lines = {
            {0, 0, cash.id, 10000, 0},
            {0, 0, sales.id, 0, 10000},
        };
        REQUIRE(db.postJournalEntry("2026-01-01", "Sale", lines));
        auto entries = db.allJournalEntries();
        REQUIRE(db.voidJournalEntry(entries[0].id));

        auto log = db.allAuditLog();
        REQUIRE(log[0].action == "VOID_JOURNAL_ENTRY");
    }

    SECTION("log is ordered most recent first") {
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        REQUIRE(db.createAccount(2000, "AP", "liability"));

        auto log = db.allAuditLog();
        REQUIRE(log.size() == 2);
        REQUIRE(log[0].id > log[1].id);
    }
}

TEST_CASE("Database backup and restore", "[db][backup]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    QString origPath = tmpDir.path() + "/original.db";
    QString backupPath = tmpDir.path() + "/backup.db";

    // Create original database with data
    {
        Database db;
        REQUIRE(db.open(origPath));
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        REQUIRE(db.createAccount(4000, "Sales", "revenue"));
        auto cash = db.accountByCode(1000);
        auto sales = db.accountByCode(4000);
        std::vector<JournalLineRow> lines = {
            {0, 0, cash.id, 50000, 0},
            {0, 0, sales.id, 0, 50000},
        };
        REQUIRE(db.postJournalEntry("2026-01-01", "Sale", lines));
        REQUIRE(db.databasePath() == origPath);
        db.close();
    }

    // Copy as backup
    REQUIRE(QFile::copy(origPath, backupPath));

    // Modify original
    {
        Database db;
        REQUIRE(db.open(origPath));
        REQUIRE(db.createAccount(2000, "AP", "liability"));
        REQUIRE(db.allAccounts().size() == 3);
        db.close();
    }

    // Restore from backup (should have only 2 accounts)
    QFile::remove(origPath);
    REQUIRE(QFile::copy(backupPath, origPath));

    {
        Database db;
        REQUIRE(db.open(origPath));
        auto accounts = db.allAccounts();
        REQUIRE(accounts.size() == 2);
        REQUIRE(db.accountByCode(1000).balanceCents == 50000);
        REQUIRE(db.accountByCode(2000).id == 0); // AP was not in backup
    }
}

TEST_CASE("Encryption key management", "[db]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());
    QString dbPath = tmpDir.path() + "/enc_test.db";

    SECTION("Open with custom key") {
        Database db;
        REQUIRE(db.open(dbPath, "my_secret_key"));
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        db.close();

        // Reopen with same key succeeds
        Database db2;
        REQUIRE(db2.open(dbPath, "my_secret_key"));
        REQUIRE(db2.accountByCode(1000).name == "Cash");
    }

    SECTION("Change encryption key") {
        Database db;
        REQUIRE(db.open(dbPath, "old_key"));
        REQUIRE(db.createAccount(1000, "Cash", "asset"));
        REQUIRE(db.changeEncryptionKey("new_key"));
        db.close();

        // Reopen with new key succeeds
        Database db2;
        REQUIRE(db2.open(dbPath, "new_key"));
        REQUIRE(db2.accountByCode(1000).name == "Cash");
    }
}

TEST_CASE("CSV import parsing", "[csv]")
{
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    SECTION("Parse basic CSV") {
        QString csvPath = tmpDir.path() + "/accounts.csv";
        {
            QFile f(csvPath);
            REQUIRE(f.open(QIODevice::WriteOnly | QIODevice::Text));
            QTextStream out(&f);
            out << "code,name,type,currency\n";
            out << "1000,Cash,asset,USD\n";
            out << "2000,Accounts Payable,liability,USD\n";
            out << "4000,\"Revenue, Grants\",revenue,EUR\n";
        }
        QString error;
        auto rows = parseCsvFile(csvPath, error);
        REQUIRE(error.isEmpty());
        REQUIRE(rows.size() == 4); // header + 3 data rows
        REQUIRE(rows[1][1] == "Cash");
        // Quoted field with embedded comma
        REQUIRE(rows[3][1] == "Revenue, Grants");
        REQUIRE(rows[3][3] == "EUR");
    }

    SECTION("Empty file") {
        QString csvPath = tmpDir.path() + "/empty.csv";
        { QFile f(csvPath); REQUIRE(f.open(QIODevice::WriteOnly)); }
        QString error;
        auto rows = parseCsvFile(csvPath, error);
        REQUIRE(error.isEmpty());
        REQUIRE(rows.empty());
    }
}
