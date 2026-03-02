#include "trial_balance_widget.h"
#include "db/database.h"
#include "accounting/account.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QToolBar>
#include <QApplication>
#include <QStyle>
#include "utils/csv_export.h"
#include "utils/print_report.h"

TrialBalanceWidget::TrialBalanceWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_table(new QTableWidget(this))
    , m_debitTotalLabel(new QLabel("0.00", this))
    , m_creditTotalLabel(new QLabel("0.00", this))
    , m_statusLabel(new QLabel(this))
{
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Code", "Account Name", "Debit", "Credit"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->verticalHeader()->hide();
    m_table->setColumnWidth(0, 70);
    m_table->setColumnWidth(2, 110);
    m_table->setColumnWidth(3, 110);

    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh", this, &TrialBalanceWidget::refresh);
    toolbar->addAction(style->standardIcon(QStyle::SP_DialogSaveButton),
        "Export CSV", this, &TrialBalanceWidget::exportCsv);
    toolbar->addAction(style->standardIcon(QStyle::SP_FileDialogDetailedView),
        "Export PDF", this, [this]() {
            printReportToPdf(m_table, "Trial Balance", this, "trial_balance.pdf");
        });

    // Totals row
    auto boldFont = m_debitTotalLabel->font();
    boldFont.setBold(true);
    m_debitTotalLabel->setFont(boldFont);
    m_creditTotalLabel->setFont(boldFont);
    m_debitTotalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_creditTotalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_debitTotalLabel->setMinimumWidth(100);
    m_creditTotalLabel->setMinimumWidth(100);

    auto *totalsLayout = new QHBoxLayout;
    totalsLayout->addStretch();
    totalsLayout->addWidget(new QLabel("Totals:", this));
    totalsLayout->addWidget(new QLabel("Debit:", this));
    totalsLayout->addWidget(m_debitTotalLabel);
    totalsLayout->addWidget(new QLabel("Credit:", this));
    totalsLayout->addWidget(m_creditTotalLabel);

    m_statusLabel->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(m_table, 1);
    layout->addLayout(totalsLayout);
    layout->addWidget(m_statusLabel);

    refresh();
}

void TrialBalanceWidget::refresh()
{
    auto accounts = m_db->allAccounts();
    m_table->setRowCount(static_cast<int>(accounts.size()));

    int64_t totalDebits = 0;
    int64_t totalCredits = 0;

    for (int i = 0; i < static_cast<int>(accounts.size()); ++i) {
        const auto &acct = accounts[static_cast<size_t>(i)];
        AccountType type = accountTypeFromString(acct.type);
        bool debitNormal = isDebitNormal(type);

        // Determine debit/credit column based on account type and balance sign
        int64_t debit = 0;
        int64_t credit = 0;

        if (debitNormal) {
            // Debit-normal accounts (Asset, Expense): positive balance is a debit
            if (acct.balanceCents >= 0) {
                debit = acct.balanceCents;
            } else {
                credit = -acct.balanceCents;
            }
        } else {
            // Credit-normal accounts (Liability, Equity, Revenue): positive balance is a credit
            if (acct.balanceCents >= 0) {
                credit = acct.balanceCents;
            } else {
                debit = -acct.balanceCents;
            }
        }

        totalDebits += debit;
        totalCredits += credit;

        auto *codeItem = new QTableWidgetItem(QString::number(acct.code));
        auto *nameItem = new QTableWidgetItem(acct.name);
        auto *debitItem = new QTableWidgetItem(
            debit > 0 ? QString::number(static_cast<double>(debit) / 100.0, 'f', 2) : QString());
        auto *creditItem = new QTableWidgetItem(
            credit > 0 ? QString::number(static_cast<double>(credit) / 100.0, 'f', 2) : QString());

        debitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        creditItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        m_table->setItem(i, 0, codeItem);
        m_table->setItem(i, 1, nameItem);
        m_table->setItem(i, 2, debitItem);
        m_table->setItem(i, 3, creditItem);
    }

    m_debitTotalLabel->setText(QString::number(static_cast<double>(totalDebits) / 100.0, 'f', 2));
    m_creditTotalLabel->setText(QString::number(static_cast<double>(totalCredits) / 100.0, 'f', 2));

    if (totalDebits == totalCredits) {
        m_statusLabel->setText("Trial Balance is in balance");
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else {
        double diff = static_cast<double>(totalDebits - totalCredits) / 100.0;
        m_statusLabel->setText(QString("OUT OF BALANCE by %1").arg(
            QString::number(qAbs(diff), 'f', 2)));
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    }
}

void TrialBalanceWidget::exportCsv()
{
    exportTableToCsv(m_table, this, "trial_balance.csv");
}
