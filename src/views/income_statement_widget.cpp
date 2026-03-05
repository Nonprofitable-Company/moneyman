#include "income_statement_widget.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QToolBar>
#include <QFont>
#include <QIcon>
#include <QStyle>
#include <QDateEdit>
#include <QCheckBox>
#include <QDate>
#include "utils/csv_export.h"
#include "utils/print_report.h"

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
    m_table->setShowGrid(true);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->verticalHeader()->hide();
    m_table->setColumnWidth(1, 130);

    auto *toolbar = new QToolBar(this);
    toolbar->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh", this, &IncomeStatementWidget::refresh);
    toolbar->addAction(QIcon(":/icons/export-csv.svg"),
        "Export CSV", this, &IncomeStatementWidget::exportCsv);
    toolbar->addAction(QIcon(":/icons/export-pdf.svg"),
        "Export PDF", this, [this]() {
            printReportToPdf(m_table, "Income Statement", this, "income_statement.pdf");
        });

    QDate today = QDate::currentDate();
    m_dateFilterCheck = new QCheckBox("Filter by period:", this);
    m_fromDate = new QDateEdit(QDate(today.year(), 1, 1), this);
    m_toDate = new QDateEdit(today, this);
    m_fromDate->setDisplayFormat("yyyy-MM-dd");
    m_toDate->setDisplayFormat("yyyy-MM-dd");
    m_fromDate->setCalendarPopup(true);
    m_toDate->setCalendarPopup(true);
    m_fromDate->setEnabled(false);
    m_toDate->setEnabled(false);
    connect(m_dateFilterCheck, &QCheckBox::toggled, m_fromDate, &QDateEdit::setEnabled);
    connect(m_dateFilterCheck, &QCheckBox::toggled, m_toDate, &QDateEdit::setEnabled);
    connect(m_dateFilterCheck, &QCheckBox::toggled, this, &IncomeStatementWidget::refresh);
    connect(m_fromDate, &QDateEdit::dateChanged, this, [this]() { if (m_dateFilterCheck->isChecked()) refresh(); });
    connect(m_toDate, &QDateEdit::dateChanged, this, [this]() { if (m_dateFilterCheck->isChecked()) refresh(); });

    auto *dateBar = new QHBoxLayout;
    dateBar->setContentsMargins(4, 0, 4, 0);
    dateBar->addWidget(m_dateFilterCheck);
    dateBar->addWidget(m_fromDate);
    dateBar->addWidget(new QLabel("to", this));
    dateBar->addWidget(m_toDate);
    dateBar->addStretch();

    auto boldFont = m_netIncomeLabel->font();
    boldFont.setBold(true);
    boldFont.setPointSize(boldFont.pointSize() + 2);
    m_netIncomeLabel->setFont(boldFont);
    m_netIncomeLabel->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addLayout(dateBar);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_netIncomeLabel);

    refresh();
}

void IncomeStatementWidget::refresh()
{
    struct AcctInfo { int code; QString name; int64_t balance; };
    std::vector<AcctInfo> revenue;
    std::vector<AcctInfo> expenses;

    if (m_dateFilterCheck->isChecked()) {
        auto balances = m_db->accountBalancesForPeriod(
            m_fromDate->date().toString("yyyy-MM-dd"),
            m_toDate->date().toString("yyyy-MM-dd"));
        for (const auto &ab : balances) {
            if (ab.type == "revenue") revenue.push_back({ab.code, ab.name, ab.balanceCents});
            else if (ab.type == "expense") expenses.push_back({ab.code, ab.name, ab.balanceCents});
        }
    } else {
        auto accounts = m_db->allAccounts();
        for (const auto &acct : accounts) {
            if (acct.type == "revenue") revenue.push_back({acct.code, acct.name, acct.balanceCents});
            else if (acct.type == "expense") expenses.push_back({acct.code, acct.name, acct.balanceCents});
        }
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
        auto *amtItem = new QTableWidgetItem(formatCents(acct.balance));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, amtItem);
        totalRevenue += acct.balance;
        ++row;
    }

    setBoldRow(row++, "Total Revenue", formatCents(totalRevenue));
    m_table->setItem(row++, 0, new QTableWidgetItem(""));

    // Expense section
    setBoldRow(row++, "EXPENSES", "");

    int64_t totalExpenses = 0;
    for (const auto &acct : expenses) {
        auto *nameItem = new QTableWidgetItem(QString("  %1 — %2").arg(acct.code).arg(acct.name));
        auto *amtItem = new QTableWidgetItem(formatCents(acct.balance));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, amtItem);
        totalExpenses += acct.balance;
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
    m_netIncomeLabel->setObjectName(netIncome >= 0 ? "statusSuccess" : "statusDanger");
    m_netIncomeLabel->style()->unpolish(m_netIncomeLabel);
    m_netIncomeLabel->style()->polish(m_netIncomeLabel);
}

void IncomeStatementWidget::exportCsv()
{
    exportTableToCsv(m_table, this, "income_statement.csv");
}
