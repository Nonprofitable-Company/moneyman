#include "income_statement_widget.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QToolBar>
#include <QFont>

IncomeStatementWidget::IncomeStatementWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_table(new QTableWidget(this))
    , m_netIncomeLabel(new QLabel(this))
{
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"Account", "Amount"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->hide();
    m_table->setColumnWidth(0, 300);

    auto *toolbar = new QToolBar(this);
    toolbar->addAction("Refresh", this, &IncomeStatementWidget::refresh);

    auto boldFont = m_netIncomeLabel->font();
    boldFont.setBold(true);
    boldFont.setPointSize(boldFont.pointSize() + 2);
    m_netIncomeLabel->setFont(boldFont);
    m_netIncomeLabel->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_netIncomeLabel);

    refresh();
}

void IncomeStatementWidget::refresh()
{
    auto accounts = m_db->allAccounts();

    // Separate revenue and expense accounts
    std::vector<AccountRow> revenue;
    std::vector<AccountRow> expenses;
    for (const auto &acct : accounts) {
        if (acct.type == "revenue") revenue.push_back(acct);
        else if (acct.type == "expense") expenses.push_back(acct);
    }

    // Row count: revenue header + revenues + revenue total + blank + expense header + expenses + expense total + blank + net income
    int rowCount = 1 + static_cast<int>(revenue.size()) + 1 + 1
                 + 1 + static_cast<int>(expenses.size()) + 1 + 1 + 1;
    m_table->setRowCount(rowCount);
    int row = 0;

    auto setBoldRow = [&](int r, const QString &label, const QString &amount) {
        auto *labelItem = new QTableWidgetItem(label);
        auto *amountItem = new QTableWidgetItem(amount);
        auto font = labelItem->font();
        font.setBold(true);
        labelItem->setFont(font);
        amountItem->setFont(font);
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(r, 0, labelItem);
        m_table->setItem(r, 1, amountItem);
    };

    auto formatCents = [](int64_t cents) -> QString {
        return QString::number(static_cast<double>(cents) / 100.0, 'f', 2);
    };

    // Revenue section
    setBoldRow(row++, "REVENUE", "");

    int64_t totalRevenue = 0;
    for (const auto &acct : revenue) {
        auto *nameItem = new QTableWidgetItem(QString("  %1 — %2").arg(acct.code).arg(acct.name));
        auto *amtItem = new QTableWidgetItem(formatCents(acct.balanceCents));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, amtItem);
        totalRevenue += acct.balanceCents;
        ++row;
    }

    setBoldRow(row++, "Total Revenue", formatCents(totalRevenue));
    m_table->setItem(row++, 0, new QTableWidgetItem(""));

    // Expense section
    setBoldRow(row++, "EXPENSES", "");

    int64_t totalExpenses = 0;
    for (const auto &acct : expenses) {
        auto *nameItem = new QTableWidgetItem(QString("  %1 — %2").arg(acct.code).arg(acct.name));
        auto *amtItem = new QTableWidgetItem(formatCents(acct.balanceCents));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, amtItem);
        totalExpenses += acct.balanceCents;
        ++row;
    }

    setBoldRow(row++, "Total Expenses", formatCents(totalExpenses));
    m_table->setItem(row++, 0, new QTableWidgetItem(""));

    // Net Income
    int64_t netIncome = totalRevenue - totalExpenses;
    setBoldRow(row, "NET INCOME", formatCents(netIncome));

    // Update summary label
    QString status = netIncome >= 0 ? "Net Income" : "Net Loss";
    m_netIncomeLabel->setText(QString("%1: $%2").arg(status).arg(formatCents(qAbs(netIncome))));
    m_netIncomeLabel->setStyleSheet(netIncome >= 0
        ? "color: green; font-weight: bold;"
        : "color: red; font-weight: bold;");
}
