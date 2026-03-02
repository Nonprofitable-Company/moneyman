#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QVariant>

static int s_connectionCounter = 0;

Database::Database(QObject *parent)
    : QObject(parent)
    , m_connectionName(QString("moneyman_conn_%1").arg(s_connectionCounter++))
{
}

Database::~Database()
{
    close();
}

bool Database::open(const QString &path, const QString &encryptionKey)
{
    if (path.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        m_dbPath = dataDir + "/moneyman.db";
    } else {
        m_dbPath = path;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    // Enable SQLCipher encryption via PRAGMA
    QString key = encryptionKey.isEmpty() ? "CHANGEME_DEFAULT_KEY" : encryptionKey;
    if (!setEncryptionKey(key)) {
        return false;
    }

    // Enable WAL mode for better concurrency
    QSqlQuery walQuery(m_db);
    walQuery.exec("PRAGMA journal_mode=WAL");

    // Enable foreign keys
    QSqlQuery fkQuery(m_db);
    fkQuery.exec("PRAGMA foreign_keys=ON");

    if (!createSchema()) {
        return false;
    }

    return true;
}

bool Database::changeEncryptionKey(const QString &newKey)
{
    if (!m_db.isOpen()) {
        m_lastError = "Database is not open";
        return false;
    }
    QSqlQuery query(m_db);
    if (!query.exec(QString("PRAGMA rekey='%1'").arg(newKey))) {
        m_lastError = "Failed to change encryption key: " + query.lastError().text();
        return false;
    }
    logAudit("CHANGE_KEY", "Encryption key changed");
    return true;
}

void Database::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::isOpen() const
{
    return m_db.isOpen();
}

QString Database::lastError() const
{
    return m_lastError;
}

bool Database::setEncryptionKey(const QString &key)
{
    QSqlQuery query(m_db);
    // SQLCipher PRAGMA key must be the first statement after opening
    if (!query.exec(QString("PRAGMA key='%1'").arg(key))) {
        m_lastError = "Failed to set encryption key: " + query.lastError().text();
        return false;
    }
    return true;
}

bool Database::createSchema()
{
    QSqlQuery query(m_db);

    const QString accountsTable = R"(
        CREATE TABLE IF NOT EXISTS accounts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            code INTEGER UNIQUE NOT NULL,
            name TEXT NOT NULL,
            type TEXT NOT NULL CHECK(type IN ('asset','liability','equity','revenue','expense')),
            currency TEXT NOT NULL DEFAULT 'USD',
            balance_cents INTEGER NOT NULL DEFAULT 0
        )
    )";

    if (!query.exec(accountsTable)) {
        m_lastError = "Failed to create accounts table: " + query.lastError().text();
        return false;
    }

    const QString entriesTable = R"(
        CREATE TABLE IF NOT EXISTS journal_entries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            entry_date TEXT NOT NULL,
            description TEXT NOT NULL,
            posted INTEGER NOT NULL DEFAULT 0
        )
    )";

    if (!query.exec(entriesTable)) {
        m_lastError = "Failed to create journal_entries table: " + query.lastError().text();
        return false;
    }

    const QString linesTable = R"(
        CREATE TABLE IF NOT EXISTS journal_lines (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            entry_id INTEGER NOT NULL REFERENCES journal_entries(id),
            account_id INTEGER NOT NULL REFERENCES accounts(id),
            debit_cents INTEGER NOT NULL DEFAULT 0,
            credit_cents INTEGER NOT NULL DEFAULT 0,
            CHECK(debit_cents >= 0 AND credit_cents >= 0),
            CHECK(debit_cents > 0 OR credit_cents > 0)
        )
    )";

    if (!query.exec(linesTable)) {
        m_lastError = "Failed to create journal_lines table: " + query.lastError().text();
        return false;
    }

    const QString periodsTable = R"(
        CREATE TABLE IF NOT EXISTS fiscal_periods (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            start_date TEXT NOT NULL,
            end_date TEXT NOT NULL,
            closed_at TEXT NOT NULL
        )
    )";

    if (!query.exec(periodsTable)) {
        m_lastError = "Failed to create fiscal_periods table: " + query.lastError().text();
        return false;
    }

    const QString templatesTable = R"(
        CREATE TABLE IF NOT EXISTS journal_templates (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            description TEXT NOT NULL
        )
    )";

    if (!query.exec(templatesTable)) {
        m_lastError = "Failed to create journal_templates table: " + query.lastError().text();
        return false;
    }

    const QString templateLinesTable = R"(
        CREATE TABLE IF NOT EXISTS template_lines (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            template_id INTEGER NOT NULL REFERENCES journal_templates(id) ON DELETE CASCADE,
            account_id INTEGER NOT NULL REFERENCES accounts(id),
            debit_cents INTEGER NOT NULL DEFAULT 0,
            credit_cents INTEGER NOT NULL DEFAULT 0
        )
    )";

    if (!query.exec(templateLinesTable)) {
        m_lastError = "Failed to create template_lines table: " + query.lastError().text();
        return false;
    }

    const QString auditLogTable = R"(
        CREATE TABLE IF NOT EXISTS audit_log (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL DEFAULT (datetime('now')),
            action TEXT NOT NULL,
            details TEXT NOT NULL
        )
    )";

    if (!query.exec(auditLogTable)) {
        m_lastError = "Failed to create audit_log table: " + query.lastError().text();
        return false;
    }

    return true;
}

bool Database::createAccount(int code, const QString &name, const QString &type,
                              const QString &currency)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO accounts (code, name, type, currency) VALUES (?, ?, ?, ?)");
    query.addBindValue(code);
    query.addBindValue(name);
    query.addBindValue(type);
    query.addBindValue(currency);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    logAudit("CREATE_ACCOUNT", QString("Created account %1 — %2 (%3, %4)").arg(code).arg(name, type, currency));
    return true;
}

bool Database::updateAccount(int64_t id, const QString &name, const QString &type)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE accounts SET name = ?, type = ? WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(type);
    query.addBindValue(static_cast<qlonglong>(id));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    logAudit("UPDATE_ACCOUNT", QString("Updated account #%1: %2 (%3)").arg(id).arg(name, type));
    return true;
}

std::vector<AccountRow> Database::allAccounts() const
{
    std::vector<AccountRow> result;
    QSqlQuery query(m_db);
    query.exec("SELECT id, code, name, type, currency, balance_cents FROM accounts ORDER BY code");

    while (query.next()) {
        AccountRow row;
        row.id = query.value(0).toLongLong();
        row.code = query.value(1).toInt();
        row.name = query.value(2).toString();
        row.type = query.value(3).toString();
        row.currency = query.value(4).toString();
        row.balanceCents = query.value(5).toLongLong();
        result.push_back(row);
    }
    return result;
}

AccountRow Database::accountById(int64_t id) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id, code, name, type, currency, balance_cents FROM accounts WHERE id = ?");
    query.addBindValue(static_cast<qlonglong>(id));
    query.exec();

    AccountRow row;
    if (query.next()) {
        row.id = query.value(0).toLongLong();
        row.code = query.value(1).toInt();
        row.name = query.value(2).toString();
        row.type = query.value(3).toString();
        row.currency = query.value(4).toString();
        row.balanceCents = query.value(5).toLongLong();
    }
    return row;
}

AccountRow Database::accountByCode(int code) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id, code, name, type, currency, balance_cents FROM accounts WHERE code = ?");
    query.addBindValue(code);
    query.exec();

    AccountRow row;
    if (query.next()) {
        row.id = query.value(0).toLongLong();
        row.code = query.value(1).toInt();
        row.name = query.value(2).toString();
        row.type = query.value(3).toString();
        row.currency = query.value(4).toString();
        row.balanceCents = query.value(5).toLongLong();
    }
    return row;
}

bool Database::postJournalEntry(const QString &date, const QString &description,
                                 const std::vector<JournalLineRow> &lines)
{
    if (lines.size() < 2) {
        m_lastError = "Journal entry must have at least 2 lines";
        return false;
    }

    // Check closed period
    if (isDateInClosedPeriod(date)) {
        m_lastError = "Cannot post to a closed fiscal period";
        return false;
    }

    // Verify debits == credits
    int64_t totalDebits = 0;
    int64_t totalCredits = 0;
    for (const auto &line : lines) {
        totalDebits += line.debitCents;
        totalCredits += line.creditCents;
    }

    if (totalDebits != totalCredits) {
        m_lastError = "Debits must equal credits";
        return false;
    }

    if (totalDebits == 0) {
        m_lastError = "Entry must have non-zero amounts";
        return false;
    }

    // Begin transaction
    m_db.transaction();

    QSqlQuery entryQuery(m_db);
    entryQuery.prepare("INSERT INTO journal_entries (entry_date, description, posted) VALUES (?, ?, 1)");
    entryQuery.addBindValue(date);
    entryQuery.addBindValue(description);

    if (!entryQuery.exec()) {
        m_lastError = entryQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    int64_t entryId = entryQuery.lastInsertId().toLongLong();

    for (const auto &line : lines) {
        QSqlQuery lineQuery(m_db);
        lineQuery.prepare("INSERT INTO journal_lines (entry_id, account_id, debit_cents, credit_cents) VALUES (?, ?, ?, ?)");
        lineQuery.addBindValue(static_cast<qlonglong>(entryId));
        lineQuery.addBindValue(static_cast<qlonglong>(line.accountId));
        lineQuery.addBindValue(static_cast<qlonglong>(line.debitCents));
        lineQuery.addBindValue(static_cast<qlonglong>(line.creditCents));

        if (!lineQuery.exec()) {
            m_lastError = lineQuery.lastError().text();
            m_db.rollback();
            return false;
        }

        // Update account balance
        // Assets & Expenses: balance += debit - credit
        // Liabilities, Equity, Revenue: balance += credit - debit
        AccountRow acct = accountById(line.accountId);
        int64_t delta = 0;
        if (acct.type == "asset" || acct.type == "expense") {
            delta = line.debitCents - line.creditCents;
        } else {
            delta = line.creditCents - line.debitCents;
        }

        QSqlQuery balQuery(m_db);
        balQuery.prepare("UPDATE accounts SET balance_cents = balance_cents + ? WHERE id = ?");
        balQuery.addBindValue(static_cast<qlonglong>(delta));
        balQuery.addBindValue(static_cast<qlonglong>(line.accountId));

        if (!balQuery.exec()) {
            m_lastError = balQuery.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    m_db.commit();
    logAudit("POST_JOURNAL_ENTRY", QString("Posted entry #%1: %2 (%3)")
        .arg(entryId).arg(description, date));
    return true;
}

std::vector<JournalEntryRow> Database::allJournalEntries() const
{
    std::vector<JournalEntryRow> result;
    QSqlQuery query(m_db);
    query.exec("SELECT id, entry_date, description, posted FROM journal_entries ORDER BY id");

    while (query.next()) {
        JournalEntryRow entry;
        entry.id = query.value(0).toLongLong();
        entry.date = query.value(1).toString();
        entry.description = query.value(2).toString();
        entry.posted = query.value(3).toBool();

        QSqlQuery lineQuery(m_db);
        lineQuery.prepare("SELECT id, entry_id, account_id, debit_cents, credit_cents FROM journal_lines WHERE entry_id = ?");
        lineQuery.addBindValue(static_cast<qlonglong>(entry.id));
        lineQuery.exec();

        while (lineQuery.next()) {
            JournalLineRow line;
            line.id = lineQuery.value(0).toLongLong();
            line.entryId = lineQuery.value(1).toLongLong();
            line.accountId = lineQuery.value(2).toLongLong();
            line.debitCents = lineQuery.value(3).toLongLong();
            line.creditCents = lineQuery.value(4).toLongLong();
            entry.lines.push_back(line);
        }

        result.push_back(entry);
    }
    return result;
}

std::vector<Database::LedgerRow> Database::ledgerForAccount(int64_t accountId) const
{
    std::vector<LedgerRow> result;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT je.id, je.entry_date, je.description, jl.debit_cents, jl.credit_cents
        FROM journal_lines jl
        JOIN journal_entries je ON je.id = jl.entry_id
        WHERE jl.account_id = ? AND je.posted = 1
        ORDER BY je.entry_date, je.id
    )");
    query.addBindValue(static_cast<qlonglong>(accountId));
    query.exec();

    while (query.next()) {
        LedgerRow row;
        row.entryId = query.value(0).toLongLong();
        row.date = query.value(1).toString();
        row.description = query.value(2).toString();
        row.debitCents = query.value(3).toLongLong();
        row.creditCents = query.value(4).toLongLong();
        result.push_back(row);
    }
    return result;
}

bool Database::isDateInClosedPeriod(const QString &date) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM fiscal_periods WHERE ? >= start_date AND ? <= end_date");
    query.addBindValue(date);
    query.addBindValue(date);
    query.exec();
    if (query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

int64_t Database::ensureRetainedEarningsAccount()
{
    AccountRow existing = accountByCode(3100);
    if (existing.id != 0) {
        return existing.id;
    }

    if (!createAccount(3100, "Retained Earnings", "equity")) {
        return -1;
    }

    AccountRow created = accountByCode(3100);
    return created.id;
}

bool Database::closePeriod(const QString &startDate, const QString &endDate)
{
    // Check for overlap with existing closed periods
    QSqlQuery overlapQuery(m_db);
    overlapQuery.prepare("SELECT COUNT(*) FROM fiscal_periods WHERE start_date <= ? AND end_date >= ?");
    overlapQuery.addBindValue(endDate);
    overlapQuery.addBindValue(startDate);
    overlapQuery.exec();
    if (overlapQuery.next() && overlapQuery.value(0).toInt() > 0) {
        m_lastError = "Period overlaps with an existing closed period";
        return false;
    }

    int64_t retainedEarningsId = ensureRetainedEarningsAccount();
    if (retainedEarningsId < 0) {
        m_lastError = "Failed to create Retained Earnings account";
        return false;
    }

    auto accounts = allAccounts();

    // Sum revenue and expense balances
    int64_t totalRevenue = 0;
    int64_t totalExpenses = 0;
    std::vector<std::pair<int64_t, int64_t>> revenueAccounts;  // id, balance
    std::vector<std::pair<int64_t, int64_t>> expenseAccounts;  // id, balance

    for (const auto &acct : accounts) {
        if (acct.type == "revenue" && acct.balanceCents != 0) {
            totalRevenue += acct.balanceCents;
            revenueAccounts.push_back({acct.id, acct.balanceCents});
        } else if (acct.type == "expense" && acct.balanceCents != 0) {
            totalExpenses += acct.balanceCents;
            expenseAccounts.push_back({acct.id, acct.balanceCents});
        }
    }

    if (revenueAccounts.empty() && expenseAccounts.empty()) {
        m_lastError = "No revenue or expense balances to close";
        return false;
    }

    m_db.transaction();

    // Create closing journal entry
    QSqlQuery entryQuery(m_db);
    entryQuery.prepare("INSERT INTO journal_entries (entry_date, description, posted) VALUES (?, ?, 1)");
    entryQuery.addBindValue(endDate);
    entryQuery.addBindValue("Closing entry — period " + startDate + " to " + endDate);
    if (!entryQuery.exec()) {
        m_lastError = entryQuery.lastError().text();
        m_db.rollback();
        return false;
    }
    int64_t entryId = entryQuery.lastInsertId().toLongLong();

    auto insertLine = [&](int64_t accountId, int64_t debit, int64_t credit) -> bool {
        QSqlQuery q(m_db);
        q.prepare("INSERT INTO journal_lines (entry_id, account_id, debit_cents, credit_cents) VALUES (?, ?, ?, ?)");
        q.addBindValue(static_cast<qlonglong>(entryId));
        q.addBindValue(static_cast<qlonglong>(accountId));
        q.addBindValue(static_cast<qlonglong>(debit));
        q.addBindValue(static_cast<qlonglong>(credit));
        if (!q.exec()) {
            m_lastError = q.lastError().text();
            return false;
        }
        return true;
    };

    auto updateBalance = [&](int64_t accountId, int64_t delta) -> bool {
        QSqlQuery q(m_db);
        q.prepare("UPDATE accounts SET balance_cents = balance_cents + ? WHERE id = ?");
        q.addBindValue(static_cast<qlonglong>(delta));
        q.addBindValue(static_cast<qlonglong>(accountId));
        if (!q.exec()) {
            m_lastError = q.lastError().text();
            return false;
        }
        return true;
    };

    // Close revenue accounts: debit revenue, credit retained earnings
    // Revenue is credit-normal, so to zero it we debit it
    for (const auto &[id, balance] : revenueAccounts) {
        if (!insertLine(id, balance, 0)) { m_db.rollback(); return false; }
        if (!updateBalance(id, -balance)) { m_db.rollback(); return false; }
    }

    // Close expense accounts: credit expense, debit retained earnings
    // Expense is debit-normal, so to zero it we credit it
    for (const auto &[id, balance] : expenseAccounts) {
        if (!insertLine(id, 0, balance)) { m_db.rollback(); return false; }
        if (!updateBalance(id, -balance)) { m_db.rollback(); return false; }
    }

    // Net income to retained earnings
    int64_t netIncome = totalRevenue - totalExpenses;
    if (netIncome > 0) {
        // Credit retained earnings (equity is credit-normal)
        if (!insertLine(retainedEarningsId, 0, netIncome)) { m_db.rollback(); return false; }
        if (!updateBalance(retainedEarningsId, netIncome)) { m_db.rollback(); return false; }
    } else if (netIncome < 0) {
        // Debit retained earnings (net loss)
        if (!insertLine(retainedEarningsId, -netIncome, 0)) { m_db.rollback(); return false; }
        if (!updateBalance(retainedEarningsId, netIncome)) { m_db.rollback(); return false; }
    }

    // Record the closed period
    QSqlQuery periodQuery(m_db);
    periodQuery.prepare("INSERT INTO fiscal_periods (start_date, end_date, closed_at) VALUES (?, ?, datetime('now'))");
    periodQuery.addBindValue(startDate);
    periodQuery.addBindValue(endDate);
    if (!periodQuery.exec()) {
        m_lastError = periodQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    m_db.commit();
    logAudit("CLOSE_PERIOD", QString("Closed period %1 to %2").arg(startDate, endDate));
    return true;
}

std::vector<Database::FiscalPeriod> Database::allFiscalPeriods() const
{
    std::vector<FiscalPeriod> result;
    QSqlQuery query(m_db);
    query.exec("SELECT id, start_date, end_date, closed_at FROM fiscal_periods ORDER BY start_date");
    while (query.next()) {
        FiscalPeriod fp;
        fp.id = query.value(0).toLongLong();
        fp.startDate = query.value(1).toString();
        fp.endDate = query.value(2).toString();
        fp.closedAt = query.value(3).toString();
        result.push_back(fp);
    }
    return result;
}

JournalEntryRow Database::journalEntryById(int64_t id) const
{
    JournalEntryRow entry;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, entry_date, description, posted FROM journal_entries WHERE id = ?");
    query.addBindValue(static_cast<qlonglong>(id));
    query.exec();

    if (query.next()) {
        entry.id = query.value(0).toLongLong();
        entry.date = query.value(1).toString();
        entry.description = query.value(2).toString();
        entry.posted = query.value(3).toBool();

        QSqlQuery lineQuery(m_db);
        lineQuery.prepare("SELECT id, entry_id, account_id, debit_cents, credit_cents FROM journal_lines WHERE entry_id = ?");
        lineQuery.addBindValue(static_cast<qlonglong>(entry.id));
        lineQuery.exec();

        while (lineQuery.next()) {
            JournalLineRow line;
            line.id = lineQuery.value(0).toLongLong();
            line.entryId = lineQuery.value(1).toLongLong();
            line.accountId = lineQuery.value(2).toLongLong();
            line.debitCents = lineQuery.value(3).toLongLong();
            line.creditCents = lineQuery.value(4).toLongLong();
            entry.lines.push_back(line);
        }
    }
    return entry;
}

bool Database::voidJournalEntry(int64_t entryId)
{
    JournalEntryRow original = journalEntryById(entryId);
    if (original.id == 0) {
        m_lastError = "Journal entry not found";
        return false;
    }

    if (!original.posted) {
        m_lastError = "Entry is not posted";
        return false;
    }

    if (original.description.startsWith("[VOIDED]")) {
        m_lastError = "Entry is already voided";
        return false;
    }

    // Build reversed lines (swap debits and credits)
    std::vector<JournalLineRow> reversedLines;
    for (const auto &line : original.lines) {
        JournalLineRow rev;
        rev.accountId = line.accountId;
        rev.debitCents = line.creditCents;
        rev.creditCents = line.debitCents;
        reversedLines.push_back(rev);
    }

    QString reversalDesc = QString("[VOID] Reversal of entry #%1: %2")
        .arg(entryId).arg(original.description);

    if (!postJournalEntry(original.date, reversalDesc, reversedLines)) {
        return false;
    }

    // Mark original as voided
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare("UPDATE journal_entries SET description = ? WHERE id = ?");
    updateQuery.addBindValue("[VOIDED] " + original.description);
    updateQuery.addBindValue(static_cast<qlonglong>(entryId));
    if (!updateQuery.exec()) {
        m_lastError = updateQuery.lastError().text();
        return false;
    }

    logAudit("VOID_JOURNAL_ENTRY", QString("Voided entry #%1: %2").arg(entryId).arg(original.description));
    return true;
}

bool Database::saveTemplate(const QString &name, const QString &description,
                             const std::vector<JournalLineRow> &lines)
{
    if (lines.size() < 2) {
        m_lastError = "Template must have at least 2 lines";
        return false;
    }

    m_db.transaction();

    QSqlQuery tplQuery(m_db);
    tplQuery.prepare("INSERT INTO journal_templates (name, description) VALUES (?, ?)");
    tplQuery.addBindValue(name);
    tplQuery.addBindValue(description);

    if (!tplQuery.exec()) {
        m_lastError = tplQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    int64_t tplId = tplQuery.lastInsertId().toLongLong();

    for (const auto &line : lines) {
        QSqlQuery lineQuery(m_db);
        lineQuery.prepare("INSERT INTO template_lines (template_id, account_id, debit_cents, credit_cents) VALUES (?, ?, ?, ?)");
        lineQuery.addBindValue(static_cast<qlonglong>(tplId));
        lineQuery.addBindValue(static_cast<qlonglong>(line.accountId));
        lineQuery.addBindValue(static_cast<qlonglong>(line.debitCents));
        lineQuery.addBindValue(static_cast<qlonglong>(line.creditCents));

        if (!lineQuery.exec()) {
            m_lastError = lineQuery.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    m_db.commit();
    logAudit("SAVE_TEMPLATE", QString("Saved template: %1").arg(name));
    return true;
}

std::vector<Database::TemplateRow> Database::allTemplates() const
{
    std::vector<TemplateRow> result;
    QSqlQuery query(m_db);
    query.exec("SELECT id, name, description FROM journal_templates ORDER BY name");

    while (query.next()) {
        TemplateRow tpl;
        tpl.id = query.value(0).toLongLong();
        tpl.name = query.value(1).toString();
        tpl.description = query.value(2).toString();

        QSqlQuery lineQuery(m_db);
        lineQuery.prepare("SELECT id, template_id, account_id, debit_cents, credit_cents FROM template_lines WHERE template_id = ?");
        lineQuery.addBindValue(static_cast<qlonglong>(tpl.id));
        lineQuery.exec();

        while (lineQuery.next()) {
            JournalLineRow line;
            line.id = lineQuery.value(0).toLongLong();
            line.entryId = lineQuery.value(1).toLongLong();
            line.accountId = lineQuery.value(2).toLongLong();
            line.debitCents = lineQuery.value(3).toLongLong();
            line.creditCents = lineQuery.value(4).toLongLong();
            tpl.lines.push_back(line);
        }

        result.push_back(tpl);
    }
    return result;
}

Database::TemplateRow Database::templateById(int64_t id) const
{
    TemplateRow tpl;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, description FROM journal_templates WHERE id = ?");
    query.addBindValue(static_cast<qlonglong>(id));
    query.exec();

    if (query.next()) {
        tpl.id = query.value(0).toLongLong();
        tpl.name = query.value(1).toString();
        tpl.description = query.value(2).toString();

        QSqlQuery lineQuery(m_db);
        lineQuery.prepare("SELECT id, template_id, account_id, debit_cents, credit_cents FROM template_lines WHERE template_id = ?");
        lineQuery.addBindValue(static_cast<qlonglong>(tpl.id));
        lineQuery.exec();

        while (lineQuery.next()) {
            JournalLineRow line;
            line.id = lineQuery.value(0).toLongLong();
            line.entryId = lineQuery.value(1).toLongLong();
            line.accountId = lineQuery.value(2).toLongLong();
            line.debitCents = lineQuery.value(3).toLongLong();
            line.creditCents = lineQuery.value(4).toLongLong();
            tpl.lines.push_back(line);
        }
    }
    return tpl;
}

bool Database::deleteTemplate(int64_t id)
{
    QSqlQuery lineQuery(m_db);
    lineQuery.prepare("DELETE FROM template_lines WHERE template_id = ?");
    lineQuery.addBindValue(static_cast<qlonglong>(id));
    lineQuery.exec();

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM journal_templates WHERE id = ?");
    query.addBindValue(static_cast<qlonglong>(id));
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    logAudit("DELETE_TEMPLATE", QString("Deleted template #%1").arg(id));
    return true;
}

void Database::logAudit(const QString &action, const QString &details)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO audit_log (action, details) VALUES (?, ?)");
    query.addBindValue(action);
    query.addBindValue(details);
    query.exec();
}

std::vector<Database::AuditLogEntry> Database::allAuditLog() const
{
    std::vector<AuditLogEntry> result;
    QSqlQuery query(m_db);
    query.exec("SELECT id, timestamp, action, details FROM audit_log ORDER BY id DESC");
    while (query.next()) {
        AuditLogEntry entry;
        entry.id = query.value(0).toLongLong();
        entry.timestamp = query.value(1).toString();
        entry.action = query.value(2).toString();
        entry.details = query.value(3).toString();
        result.push_back(entry);
    }
    return result;
}
