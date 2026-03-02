#include "balance_sheet_widget.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QToolBar>
#include <QApplication>
#include <QStyle>

BalanceSheetWidget::BalanceSheetWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_table(new QTableWidget(this))
    , m_statusLabel(new QLabel(this))
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

    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh", this, &BalanceSheetWidget::refresh);

    m_statusLabel->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_statusLabel);

    refresh();
}

void BalanceSheetWidget::refresh()
{
    auto accounts = m_db->allAccounts();

    std::vector<AccountRow> assets;
    std::vector<AccountRow> liabilities;
    std::vector<AccountRow> equity;
    int64_t totalRevenue = 0;
    int64_t totalExpenses = 0;

    for (const auto &acct : accounts) {
        if (acct.type == "asset") assets.push_back(acct);
        else if (acct.type == "liability") liabilities.push_back(acct);
        else if (acct.type == "equity") equity.push_back(acct);
        else if (acct.type == "revenue") totalRevenue += acct.balanceCents;
        else if (acct.type == "expense") totalExpenses += acct.balanceCents;
    }

    int64_t netIncome = totalRevenue - totalExpenses;

    auto formatCents = [](int64_t cents) -> QString {
        return QString::number(static_cast<double>(cents) / 100.0, 'f', 2);
    };

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

    // Rows: assets header + assets + total + blank + liab header + liabs + total + blank
    //       + equity header + equities + net income row + total + blank + total L+E
    int rowCount = 1 + static_cast<int>(assets.size()) + 1 + 1
                 + 1 + static_cast<int>(liabilities.size()) + 1 + 1
                 + 1 + static_cast<int>(equity.size()) + 1 + 1 + 1 + 1;
    m_table->setRowCount(rowCount);
    int row = 0;

    // Assets
    setBoldRow(row++, "ASSETS", "");
    int64_t totalAssets = 0;
    for (const auto &acct : assets) {
        auto *nameItem = new QTableWidgetItem(QString("  %1 — %2").arg(acct.code).arg(acct.name));
        auto *amtItem = new QTableWidgetItem(formatCents(acct.balanceCents));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, amtItem);
        totalAssets += acct.balanceCents;
        ++row;
    }
    setBoldRow(row++, "Total Assets", formatCents(totalAssets));
    m_table->setItem(row++, 0, new QTableWidgetItem(""));

    // Liabilities
    setBoldRow(row++, "LIABILITIES", "");
    int64_t totalLiabilities = 0;
    for (const auto &acct : liabilities) {
        auto *nameItem = new QTableWidgetItem(QString("  %1 — %2").arg(acct.code).arg(acct.name));
        auto *amtItem = new QTableWidgetItem(formatCents(acct.balanceCents));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, amtItem);
        totalLiabilities += acct.balanceCents;
        ++row;
    }
    setBoldRow(row++, "Total Liabilities", formatCents(totalLiabilities));
    m_table->setItem(row++, 0, new QTableWidgetItem(""));

    // Equity (with net income folded in)
    setBoldRow(row++, "EQUITY", "");
    int64_t totalEquity = 0;
    for (const auto &acct : equity) {
        auto *nameItem = new QTableWidgetItem(QString("  %1 — %2").arg(acct.code).arg(acct.name));
        auto *amtItem = new QTableWidgetItem(formatCents(acct.balanceCents));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, amtItem);
        totalEquity += acct.balanceCents;
        ++row;
    }
    // Net income as a line item under equity
    {
        auto *nameItem = new QTableWidgetItem("  Net Income (Current Period)");
        auto *amtItem = new QTableWidgetItem(formatCents(netIncome));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        auto font = nameItem->font();
        font.setItalic(true);
        nameItem->setFont(font);
        amtItem->setFont(font);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, amtItem);
        ++row;
    }
    totalEquity += netIncome;
    setBoldRow(row++, "Total Equity", formatCents(totalEquity));
    m_table->setItem(row++, 0, new QTableWidgetItem(""));

    // Total Liabilities + Equity
    int64_t totalLE = totalLiabilities + totalEquity;
    setBoldRow(row, "TOTAL LIABILITIES + EQUITY", formatCents(totalLE));

    // Balance check: Assets == Liabilities + Equity
    if (totalAssets == totalLE) {
        m_statusLabel->setText("Assets = Liabilities + Equity (Balanced)");
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else {
        double diff = static_cast<double>(totalAssets - totalLE) / 100.0;
        m_statusLabel->setText(QString("OUT OF BALANCE by %1").arg(
            QString::number(qAbs(diff), 'f', 2)));
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    }
}
