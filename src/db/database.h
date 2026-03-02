#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <cstdint>
#include <vector>

struct AccountRow {
    int64_t id = 0;
    int code = 0;
    QString name;
    QString type; // asset, liability, equity, revenue, expense
    QString currency = "USD";
    int64_t balanceCents = 0;
};

struct JournalLineRow {
    int64_t id = 0;
    int64_t entryId = 0;
    int64_t accountId = 0;
    int64_t debitCents = 0;
    int64_t creditCents = 0;
};

struct JournalEntryRow {
    int64_t id = 0;
    QString date;
    QString description;
    bool posted = false;
    std::vector<JournalLineRow> lines;
};

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database() override;

    bool open(const QString &path = QString());
    void close();
    bool isOpen() const;
    QString lastError() const;

    // Accounts
    bool createAccount(int code, const QString &name, const QString &type,
                       const QString &currency = "USD");
    bool updateAccount(int64_t id, const QString &name, const QString &type);
    std::vector<AccountRow> allAccounts() const;
    AccountRow accountById(int64_t id) const;
    AccountRow accountByCode(int code) const;

    // Journal entries
    bool postJournalEntry(const QString &date, const QString &description,
                          const std::vector<JournalLineRow> &lines);
    std::vector<JournalEntryRow> allJournalEntries() const;
    JournalEntryRow journalEntryById(int64_t id) const;
    bool voidJournalEntry(int64_t entryId);

    // Ledger: lines for a specific account, joined with entry date/description
    struct LedgerRow {
        int64_t entryId = 0;
        QString date;
        QString description;
        int64_t debitCents = 0;
        int64_t creditCents = 0;
    };
    std::vector<LedgerRow> ledgerForAccount(int64_t accountId) const;

    // Fiscal periods
    struct FiscalPeriod {
        int64_t id = 0;
        QString startDate;
        QString endDate;
        QString closedAt;
    };
    bool isDateInClosedPeriod(const QString &date) const;
    bool closePeriod(const QString &startDate, const QString &endDate);
    std::vector<FiscalPeriod> allFiscalPeriods() const;
    int64_t ensureRetainedEarningsAccount();

    // Journal templates (recurring entries)
    struct TemplateRow {
        int64_t id = 0;
        QString name;
        QString description;
        std::vector<JournalLineRow> lines;
    };
    bool saveTemplate(const QString &name, const QString &description,
                      const std::vector<JournalLineRow> &lines);
    std::vector<TemplateRow> allTemplates() const;
    TemplateRow templateById(int64_t id) const;
    bool deleteTemplate(int64_t id);

    // Audit log
    struct AuditLogEntry {
        int64_t id = 0;
        QString timestamp;
        QString action;
        QString details;
    };
    void logAudit(const QString &action, const QString &details);
    std::vector<AuditLogEntry> allAuditLog() const;

private:
    bool createSchema();
    bool setEncryptionKey(const QString &key);

    QSqlDatabase m_db;
    QString m_lastError;
    QString m_dbPath;
    QString m_connectionName;
};

#endif // DATABASE_H
