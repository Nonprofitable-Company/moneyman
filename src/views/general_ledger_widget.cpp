#include "general_ledger_widget.h"
#include "db/database.h"
#include "accounting/account.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QToolBar>
#include <QLabel>
#include <QApplication>
#include <QStyle>

GeneralLedgerWidget::GeneralLedgerWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_accountCombo(new QComboBox(this))
    , m_table(new QTableWidget(this))
{
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"Date", "Description", "Debit", "Credit", "Balance"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->verticalHeader()->hide();
    m_table->setColumnWidth(0, 90);
    m_table->setColumnWidth(2, 100);
    m_table->setColumnWidth(3, 100);
    m_table->setColumnWidth(4, 110);

    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addWidget(new QLabel(" Account: ", this));
    toolbar->addWidget(m_accountCombo);
    toolbar->addAction(style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh", this, &GeneralLedgerWidget::refresh);

    connect(m_accountCombo, &QComboBox::currentIndexChanged,
            this, &GeneralLedgerWidget::onAccountChanged);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(m_table, 1);

    populateAccountSelector();
}

void GeneralLedgerWidget::refresh()
{
    populateAccountSelector();
}

void GeneralLedgerWidget::populateAccountSelector()
{
    int64_t currentId = m_accountCombo->currentData().toLongLong();

    m_accountCombo->blockSignals(true);
    m_accountCombo->clear();
    m_accountCombo->addItem("-- Select Account --", static_cast<qlonglong>(0));

    auto accounts = m_db->allAccounts();
    int restoreIndex = 0;
    for (size_t i = 0; i < accounts.size(); ++i) {
        const auto &acct = accounts[i];
        QString label = QString("%1 — %2").arg(acct.code).arg(acct.name);
        m_accountCombo->addItem(label, static_cast<qlonglong>(acct.id));
        if (acct.id == currentId) {
            restoreIndex = static_cast<int>(i) + 1;
        }
    }
    m_accountCombo->setCurrentIndex(restoreIndex);
    m_accountCombo->blockSignals(false);

    onAccountChanged(restoreIndex);
}

void GeneralLedgerWidget::onAccountChanged(int index)
{
    int64_t accountId = m_accountCombo->itemData(index).toLongLong();
    if (accountId > 0) {
        loadLedger(accountId);
    } else {
        m_table->setRowCount(0);
    }
}

void GeneralLedgerWidget::loadLedger(int64_t accountId)
{
    auto rows = m_db->ledgerForAccount(accountId);
    AccountRow acct = m_db->accountById(accountId);
    AccountType type = accountTypeFromString(acct.type);
    bool debitNormal = isDebitNormal(type);

    m_table->setRowCount(static_cast<int>(rows.size()));

    int64_t runningBalance = 0;

    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
        const auto &row = rows[static_cast<size_t>(i)];

        // Running balance: debit-normal accounts increase with debits
        if (debitNormal) {
            runningBalance += row.debitCents - row.creditCents;
        } else {
            runningBalance += row.creditCents - row.debitCents;
        }

        auto *dateItem = new QTableWidgetItem(row.date);
        auto *descItem = new QTableWidgetItem(row.description);
        auto *debitItem = new QTableWidgetItem(
            row.debitCents > 0 ? QString::number(static_cast<double>(row.debitCents) / 100.0, 'f', 2) : QString());
        auto *creditItem = new QTableWidgetItem(
            row.creditCents > 0 ? QString::number(static_cast<double>(row.creditCents) / 100.0, 'f', 2) : QString());
        auto *balItem = new QTableWidgetItem(
            QString::number(static_cast<double>(runningBalance) / 100.0, 'f', 2));

        debitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        creditItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        balItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        m_table->setItem(i, 0, dateItem);
        m_table->setItem(i, 1, descItem);
        m_table->setItem(i, 2, debitItem);
        m_table->setItem(i, 3, creditItem);
        m_table->setItem(i, 4, balItem);
    }
}
