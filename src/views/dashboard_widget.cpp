#include "dashboard_widget.h"
#include "db/database.h"
#include "accounting/account.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QToolBar>
#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QTableWidget>
#include <QHeaderView>

static QString formatCents(int64_t cents)
{
    return QString::number(static_cast<double>(cents) / 100.0, 'f', 2);
}

DashboardWidget::DashboardWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
{
    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh", this, &DashboardWidget::refresh);

    // Create metric cards
    m_totalAssets = makeMetricCard("Total Assets", this);
    m_totalLiabilities = makeMetricCard("Total Liabilities", this);
    m_totalEquity = makeMetricCard("Total Equity", this);
    m_totalRevenue = makeMetricCard("Total Revenue", this);
    m_totalExpenses = makeMetricCard("Total Expenses", this);
    m_netIncome = makeMetricCard("Net Income", this);
    m_accountCount = makeMetricCard("Accounts", this);
    m_entryCount = makeMetricCard("Journal Entries", this);
    m_balanceStatus = makeMetricCard("Trial Balance", this);

    auto *grid = new QGridLayout;
    grid->setSpacing(12);

    // Row 0: Balance sheet summary
    grid->addWidget(m_totalAssets->parentWidget(), 0, 0);
    grid->addWidget(m_totalLiabilities->parentWidget(), 0, 1);
    grid->addWidget(m_totalEquity->parentWidget(), 0, 2);

    // Row 1: Income summary
    grid->addWidget(m_totalRevenue->parentWidget(), 1, 0);
    grid->addWidget(m_totalExpenses->parentWidget(), 1, 1);
    grid->addWidget(m_netIncome->parentWidget(), 1, 2);

    // Row 2: Stats
    grid->addWidget(m_accountCount->parentWidget(), 2, 0);
    grid->addWidget(m_entryCount->parentWidget(), 2, 1);
    grid->addWidget(m_balanceStatus->parentWidget(), 2, 2);

    // Recent activity tables
    m_recentEntries = new QTableWidget(this);
    m_recentEntries->setColumnCount(3);
    m_recentEntries->setHorizontalHeaderLabels({"Date", "Description", "#Lines"});
    m_recentEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentEntries->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentEntries->setAlternatingRowColors(true);
    m_recentEntries->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_recentEntries->verticalHeader()->hide();
    m_recentEntries->setMaximumHeight(180);

    m_recentAudit = new QTableWidget(this);
    m_recentAudit->setColumnCount(3);
    m_recentAudit->setHorizontalHeaderLabels({"Timestamp", "Action", "Details"});
    m_recentAudit->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentAudit->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentAudit->setAlternatingRowColors(true);
    m_recentAudit->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_recentAudit->verticalHeader()->hide();
    m_recentAudit->setMaximumHeight(180);

    auto *recentLayout = new QHBoxLayout;
    auto *entriesGroup = new QVBoxLayout;
    entriesGroup->addWidget(new QLabel("<b>Recent Journal Entries</b>", this));
    entriesGroup->addWidget(m_recentEntries);
    auto *auditGroup = new QVBoxLayout;
    auditGroup->addWidget(new QLabel("<b>Recent Audit Log</b>", this));
    auditGroup->addWidget(m_recentAudit);
    recentLayout->addLayout(entriesGroup);
    recentLayout->addLayout(auditGroup);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addLayout(grid);
    layout->addLayout(recentLayout);
    layout->addStretch();

    refresh();
}

QLabel *DashboardWidget::makeMetricCard(const QString &title, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setStyleSheet(
        "QFrame { background: #f8f9fa; border: 1px solid #dee2e6; "
        "border-radius: 6px; padding: 12px; }");

    auto *titleLabel = new QLabel(title, frame);
    titleLabel->setStyleSheet("color: #6c757d; font-size: 11px; font-weight: bold;");

    auto *valueLabel = new QLabel("--", frame);
    QFont font = valueLabel->font();
    font.setPointSize(18);
    font.setBold(true);
    valueLabel->setFont(font);
    valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *layout = new QVBoxLayout(frame);
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    return valueLabel;
}

void DashboardWidget::refresh()
{
    auto accounts = m_db->allAccounts();
    auto entries = m_db->allJournalEntries();

    int64_t assets = 0, liabilities = 0, equity = 0, revenue = 0, expenses = 0;

    for (const auto &acct : accounts) {
        AccountType type = accountTypeFromString(acct.type);
        switch (type) {
        case AccountType::Asset:    assets += acct.balanceCents; break;
        case AccountType::Liability: liabilities += acct.balanceCents; break;
        case AccountType::Equity:   equity += acct.balanceCents; break;
        case AccountType::Revenue:  revenue += acct.balanceCents; break;
        case AccountType::Expense:  expenses += acct.balanceCents; break;
        default: break;
        }
    }

    int64_t netIncome = revenue - expenses;

    m_totalAssets->setText("$" + formatCents(assets));
    m_totalLiabilities->setText("$" + formatCents(liabilities));
    m_totalEquity->setText("$" + formatCents(equity));
    m_totalRevenue->setText("$" + formatCents(revenue));
    m_totalExpenses->setText("$" + formatCents(expenses));

    if (netIncome >= 0) {
        m_netIncome->setText("$" + formatCents(netIncome));
        m_netIncome->setStyleSheet("color: green; font-size: 18pt; font-weight: bold;");
    } else {
        m_netIncome->setText("-$" + formatCents(-netIncome));
        m_netIncome->setStyleSheet("color: red; font-size: 18pt; font-weight: bold;");
    }

    m_accountCount->setText(QString::number(accounts.size()));
    m_entryCount->setText(QString::number(entries.size()));

    // Check trial balance
    int64_t totalDebits = 0, totalCredits = 0;
    for (const auto &acct : accounts) {
        AccountType type = accountTypeFromString(acct.type);
        if (isDebitNormal(type)) {
            if (acct.balanceCents >= 0) totalDebits += acct.balanceCents;
            else totalCredits += -acct.balanceCents;
        } else {
            if (acct.balanceCents >= 0) totalCredits += acct.balanceCents;
            else totalDebits += -acct.balanceCents;
        }
    }

    if (totalDebits == totalCredits) {
        m_balanceStatus->setText("In Balance");
        m_balanceStatus->setStyleSheet("color: green; font-size: 18pt; font-weight: bold;");
    } else {
        m_balanceStatus->setText("OUT OF BALANCE");
        m_balanceStatus->setStyleSheet("color: red; font-size: 18pt; font-weight: bold;");
    }

    // Recent journal entries (last 5)
    m_recentEntries->setRowCount(0);
    int entryCount = std::min(static_cast<int>(entries.size()), 5);
    for (int i = 0; i < entryCount; ++i) {
        const auto &e = entries[entries.size() - 1 - static_cast<size_t>(i)];
        int row = m_recentEntries->rowCount();
        m_recentEntries->insertRow(row);
        m_recentEntries->setItem(row, 0, new QTableWidgetItem(e.date));
        m_recentEntries->setItem(row, 1, new QTableWidgetItem(e.description));
        m_recentEntries->setItem(row, 2, new QTableWidgetItem(QString::number(e.lines.size())));
    }

    // Recent audit log (last 5)
    auto auditLog = m_db->allAuditLog();
    m_recentAudit->setRowCount(0);
    int auditCount = std::min(static_cast<int>(auditLog.size()), 5);
    for (int i = 0; i < auditCount; ++i) {
        const auto &a = auditLog[auditLog.size() - 1 - static_cast<size_t>(i)];
        int row = m_recentAudit->rowCount();
        m_recentAudit->insertRow(row);
        m_recentAudit->setItem(row, 0, new QTableWidgetItem(a.timestamp));
        m_recentAudit->setItem(row, 1, new QTableWidgetItem(a.action));
        m_recentAudit->setItem(row, 2, new QTableWidgetItem(a.details));
    }
}
