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

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addLayout(grid);
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
}
