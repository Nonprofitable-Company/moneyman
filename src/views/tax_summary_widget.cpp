#include "tax_summary_widget.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QToolBar>
#include <QFont>
#include <QApplication>
#include <QStyle>
#include <QDateEdit>
#include <QDate>
#include <QMap>
#include "utils/csv_export.h"
#include "utils/print_report.h"

TaxSummaryWidget::TaxSummaryWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_table(new QTableWidget(this))
    , m_netIncomeLabel(new QLabel(this))
{
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"Category / Account", "Amount"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->verticalHeader()->hide();
    m_table->setColumnWidth(1, 130);

    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh", this, &TaxSummaryWidget::refresh);
    toolbar->addAction(style->standardIcon(QStyle::SP_DialogSaveButton),
        "Export CSV", this, &TaxSummaryWidget::exportCsv);
    toolbar->addAction(style->standardIcon(QStyle::SP_FileDialogDetailedView),
        "Export PDF", this, [this]() {
            printReportToPdf(m_table, "Tax Summary", this, "tax_summary.pdf");
        });

    QDate today = QDate::currentDate();
    m_fromDate = new QDateEdit(QDate(today.year(), 1, 1), this);
    m_toDate = new QDateEdit(QDate(today.year(), 12, 31), this);
    m_fromDate->setDisplayFormat("yyyy-MM-dd");
    m_toDate->setDisplayFormat("yyyy-MM-dd");
    m_fromDate->setCalendarPopup(true);
    m_toDate->setCalendarPopup(true);
    connect(m_fromDate, &QDateEdit::dateChanged, this, &TaxSummaryWidget::refresh);
    connect(m_toDate, &QDateEdit::dateChanged, this, &TaxSummaryWidget::refresh);

    auto *dateBar = new QHBoxLayout;
    dateBar->setContentsMargins(4, 0, 4, 0);
    dateBar->addWidget(new QLabel("Period:", this));
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

void TaxSummaryWidget::refresh()
{
    struct AcctInfo { int code; QString name; int64_t balance; };

    // Group accounts by tax category, separated into income vs deduction
    QMap<QString, std::vector<AcctInfo>> incomeGroups;
    QMap<QString, std::vector<AcctInfo>> deductionGroups;

    auto balances = m_db->accountBalancesForPeriod(
        m_fromDate->date().toString("yyyy-MM-dd"),
        m_toDate->date().toString("yyyy-MM-dd"));

    for (const auto &ab : balances) {
        if (ab.type == "revenue") {
            QString cat = ab.taxCategory.isEmpty() ? "Uncategorized" : ab.taxCategory;
            incomeGroups[cat].push_back({ab.code, ab.name, ab.balanceCents});
        } else if (ab.type == "expense") {
            QString cat = ab.taxCategory.isEmpty() ? "Uncategorized" : ab.taxCategory;
            deductionGroups[cat].push_back({ab.code, ab.name, ab.balanceCents});
        }
    }

    // Count rows needed
    // INCOME heading + (category heading + accounts) per group + Total Income + blank
    // DEDUCTIONS heading + (category heading + accounts) per group + Total Deductions + blank
    // NET PROFIT (LOSS)
    int rowCount = 1; // INCOME
    for (auto it = incomeGroups.constBegin(); it != incomeGroups.constEnd(); ++it)
        rowCount += 1 + static_cast<int>(it.value().size());
    rowCount += 2; // Total Income + blank

    rowCount += 1; // DEDUCTIONS
    for (auto it = deductionGroups.constBegin(); it != deductionGroups.constEnd(); ++it)
        rowCount += 1 + static_cast<int>(it.value().size());
    rowCount += 2; // Total Deductions + blank

    rowCount += 1; // NET PROFIT

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

    // INCOME section
    setBoldRow(row++, "INCOME", "");

    int64_t totalIncome = 0;
    for (auto it = incomeGroups.constBegin(); it != incomeGroups.constEnd(); ++it) {
        // Category heading
        auto *catItem = new QTableWidgetItem("  " + it.key());
        auto catFont = catItem->font();
        catFont.setBold(true);
        catItem->setFont(catFont);
        m_table->setItem(row, 0, catItem);
        m_table->setItem(row, 1, new QTableWidgetItem(""));
        ++row;

        for (const auto &acct : it.value()) {
            auto *nameItem = new QTableWidgetItem(QString("    %1 — %2").arg(acct.code).arg(acct.name));
            auto *amtItem = new QTableWidgetItem(formatCents(acct.balance));
            amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(row, 0, nameItem);
            m_table->setItem(row, 1, amtItem);
            totalIncome += acct.balance;
            ++row;
        }
    }

    setBoldRow(row++, "Total Income", formatCents(totalIncome));
    m_table->setItem(row++, 0, new QTableWidgetItem(""));

    // DEDUCTIONS section
    setBoldRow(row++, "DEDUCTIONS", "");

    int64_t totalDeductions = 0;
    for (auto it = deductionGroups.constBegin(); it != deductionGroups.constEnd(); ++it) {
        auto *catItem = new QTableWidgetItem("  " + it.key());
        auto catFont = catItem->font();
        catFont.setBold(true);
        catItem->setFont(catFont);
        m_table->setItem(row, 0, catItem);
        m_table->setItem(row, 1, new QTableWidgetItem(""));
        ++row;

        for (const auto &acct : it.value()) {
            auto *nameItem = new QTableWidgetItem(QString("    %1 — %2").arg(acct.code).arg(acct.name));
            auto *amtItem = new QTableWidgetItem(formatCents(acct.balance));
            amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(row, 0, nameItem);
            m_table->setItem(row, 1, amtItem);
            totalDeductions += acct.balance;
            ++row;
        }
    }

    setBoldRow(row++, "Total Deductions", formatCents(totalDeductions));
    m_table->setItem(row++, 0, new QTableWidgetItem(""));

    // NET PROFIT (LOSS)
    // Revenue balances from accountBalancesForPeriod are negative (credits - debits)
    // so totalIncome is negative for net revenue. Negate for display.
    int64_t netProfit = -totalIncome - totalDeductions;
    setBoldRow(row, "NET PROFIT (LOSS)", formatCents(netProfit));

    // Summary label
    QString status = netProfit >= 0 ? "Net Profit" : "Net Loss";
    m_netIncomeLabel->setText(QString("%1: $%2").arg(status).arg(formatCents(qAbs(netProfit))));
    m_netIncomeLabel->setStyleSheet(netProfit >= 0
        ? "color: green; font-weight: bold;"
        : "color: red; font-weight: bold;");
}

void TaxSummaryWidget::exportCsv()
{
    exportTableToCsv(m_table, this, "tax_summary.csv");
}
