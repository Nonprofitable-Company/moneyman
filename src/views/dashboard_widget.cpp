#include "dashboard_widget.h"
#include "db/database.h"
#include "accounting/account.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QToolBar>
#include <QFont>
#include <QTableWidget>
#include <QHeaderView>
#include <QIcon>
#include <QStyle>

static QString formatCents(int64_t cents)
{
    return QString::number(static_cast<double>(cents) / 100.0, 'f', 2);
}

DashboardWidget::DashboardWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
{
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh", this, &DashboardWidget::refresh);

    m_totalAssets = makeMetricCard("Total Assets");
    m_totalLiabilities = makeMetricCard("Total Liabilities");
    m_totalEquity = makeMetricCard("Total Equity");
    m_totalRevenue = makeMetricCard("Total Revenue");
    m_totalExpenses = makeMetricCard("Total Expenses");
    m_netIncome = makeMetricCard("Net Income", true);
    m_accountCount = makeMetricCard("Accounts");
    m_entryCount = makeMetricCard("Journal Entries");
    m_balanceStatus = makeMetricCard("Trial Balance");

    auto *grid = new QGridLayout;
    grid->setSpacing(16);

    grid->addWidget(m_totalAssets.frame, 0, 0);
    grid->addWidget(m_totalLiabilities.frame, 0, 1);
    grid->addWidget(m_totalEquity.frame, 0, 2);

    grid->addWidget(m_totalRevenue.frame, 1, 0);
    grid->addWidget(m_totalExpenses.frame, 1, 1);
    grid->addWidget(m_netIncome.frame, 1, 2);

    grid->addWidget(m_accountCount.frame, 2, 0);
    grid->addWidget(m_entryCount.frame, 2, 1);
    grid->addWidget(m_balanceStatus.frame, 2, 2);

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
    auto *entriesHeader = new QLabel("Recent Journal Entries", this);
    entriesHeader->setObjectName("sectionHeader");
    entriesGroup->addWidget(entriesHeader);
    entriesGroup->addWidget(m_recentEntries);
    auto *auditGroup = new QVBoxLayout;
    auto *auditHeader = new QLabel("Recent Audit Log", this);
    auditHeader->setObjectName("sectionHeader");
    auditGroup->addWidget(auditHeader);
    auditGroup->addWidget(m_recentAudit);
    recentLayout->addLayout(entriesGroup);
    recentLayout->addLayout(auditGroup);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 16);
    layout->addWidget(toolbar);
    layout->addLayout(grid);
    layout->addLayout(recentLayout);
    layout->addStretch();

    refresh();
}

DashboardWidget::MetricCard DashboardWidget::makeMetricCard(
    const QString &title, bool accent)
{
    MetricCard card;
    card.frame = new QFrame(this);
    card.frame->setObjectName(accent ? "metricCardAccent" : "metricCard");

    card.title = new QLabel(title, card.frame);
    card.title->setObjectName("metricTitle");

    card.value = new QLabel("--", card.frame);
    card.value->setObjectName("metricValue");
    card.value->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *layout = new QVBoxLayout(card.frame);
    layout->addWidget(card.title);
    layout->addWidget(card.value);

    return card;
}

void DashboardWidget::refresh()
{
    auto accounts = m_db->allAccounts();
    auto entries = m_db->allJournalEntries();

    int64_t assets = 0, liabilities = 0, equity = 0, revenue = 0, expenses = 0;

    for (const auto &acct : accounts) {
        AccountType type = accountTypeFromString(acct.type);
        switch (type) {
        case AccountType::Asset:     assets += acct.balanceCents; break;
        case AccountType::Liability: liabilities += acct.balanceCents; break;
        case AccountType::Equity:    equity += acct.balanceCents; break;
        case AccountType::Revenue:   revenue += acct.balanceCents; break;
        case AccountType::Expense:   expenses += acct.balanceCents; break;
        default: break;
        }
    }

    int64_t netIncome = revenue - expenses;

    m_totalAssets.value->setText("$" + formatCents(assets));
    m_totalLiabilities.value->setText("$" + formatCents(liabilities));
    m_totalEquity.value->setText("$" + formatCents(equity));
    m_totalRevenue.value->setText("$" + formatCents(revenue));
    m_totalExpenses.value->setText("$" + formatCents(expenses));

    if (netIncome >= 0) {
        m_netIncome.value->setText("$" + formatCents(netIncome));
        m_netIncome.value->setObjectName("metricValueSuccess");
    } else {
        m_netIncome.value->setText("-$" + formatCents(-netIncome));
        m_netIncome.value->setObjectName("metricValueDanger");
    }
    m_netIncome.value->style()->unpolish(m_netIncome.value);
    m_netIncome.value->style()->polish(m_netIncome.value);

    m_accountCount.value->setText(QString::number(accounts.size()));
    m_entryCount.value->setText(QString::number(entries.size()));

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
        m_balanceStatus.value->setText("In Balance");
        m_balanceStatus.value->setObjectName("metricValueSuccess");
    } else {
        m_balanceStatus.value->setText("OUT OF BALANCE");
        m_balanceStatus.value->setObjectName("metricValueDanger");
    }
    m_balanceStatus.value->style()->unpolish(m_balanceStatus.value);
    m_balanceStatus.value->style()->polish(m_balanceStatus.value);

    m_recentEntries->setRowCount(0);
    int entryCount = std::min(static_cast<int>(entries.size()), 5);
    for (int i = 0; i < entryCount; ++i) {
        const auto &e = entries[entries.size() - 1 - static_cast<size_t>(i)];
        int row = m_recentEntries->rowCount();
        m_recentEntries->insertRow(row);
        m_recentEntries->setItem(row, 0, new QTableWidgetItem(e.date));
        m_recentEntries->setItem(row, 1, new QTableWidgetItem(e.description));
        m_recentEntries->setItem(row, 2,
            new QTableWidgetItem(QString::number(e.lines.size())));
    }

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
