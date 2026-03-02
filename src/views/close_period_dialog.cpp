#include "close_period_dialog.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDateEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDialogButtonBox>

ClosePeriodDialog::ClosePeriodDialog(Database *db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
    , m_startDate(new QDateEdit(this))
    , m_endDate(new QDateEdit(this))
    , m_summaryLabel(new QLabel(this))
{
    setWindowTitle("Close Fiscal Period");
    setMinimumWidth(400);

    m_startDate->setCalendarPopup(true);
    m_endDate->setCalendarPopup(true);
    m_startDate->setDisplayFormat("yyyy-MM-dd");
    m_endDate->setDisplayFormat("yyyy-MM-dd");

    // Default to current year
    QDate today = QDate::currentDate();
    m_startDate->setDate(QDate(today.year(), 1, 1));
    m_endDate->setDate(QDate(today.year(), 12, 31));

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Period Start:", m_startDate);
    formLayout->addRow("Period End:", m_endDate);

    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setStyleSheet("padding: 8px; background: #f0f0f0; border-radius: 4px;");

    auto *closeBtn = new QPushButton("Close Period");
    closeBtn->setDefault(true);
    auto *cancelBtn = new QPushButton("Cancel");

    auto *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(closeBtn, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(cancelBtn, QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ClosePeriodDialog::onClose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_startDate, &QDateEdit::dateChanged, this, &ClosePeriodDialog::updateSummary);
    connect(m_endDate, &QDateEdit::dateChanged, this, &ClosePeriodDialog::updateSummary);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(new QLabel("This will create a closing journal entry that zeros all "
                                  "revenue and expense account balances into Retained Earnings "
                                  "(account 3100)."));
    layout->addWidget(m_summaryLabel);
    layout->addWidget(buttonBox);

    updateSummary();
}

void ClosePeriodDialog::updateSummary()
{
    auto accounts = m_db->allAccounts();
    int64_t totalRevenue = 0;
    int64_t totalExpenses = 0;
    int revenueCount = 0;
    int expenseCount = 0;

    for (const auto &acct : accounts) {
        if (acct.type == "revenue" && acct.balanceCents != 0) {
            totalRevenue += acct.balanceCents;
            ++revenueCount;
        } else if (acct.type == "expense" && acct.balanceCents != 0) {
            totalExpenses += acct.balanceCents;
            ++expenseCount;
        }
    }

    int64_t netIncome = totalRevenue - totalExpenses;
    auto fmt = [](int64_t cents) { return QString::number(static_cast<double>(cents) / 100.0, 'f', 2); };

    m_summaryLabel->setText(
        QString("Revenue accounts to close: %1 (total %2)\n"
                "Expense accounts to close: %3 (total %4)\n"
                "Net income to Retained Earnings: %5")
            .arg(revenueCount).arg(fmt(totalRevenue))
            .arg(expenseCount).arg(fmt(totalExpenses))
            .arg(fmt(netIncome)));
}

void ClosePeriodDialog::onClose()
{
    QString start = m_startDate->date().toString("yyyy-MM-dd");
    QString end = m_endDate->date().toString("yyyy-MM-dd");

    if (start >= end) {
        QMessageBox::warning(this, "Invalid Period", "Start date must be before end date.");
        return;
    }

    auto reply = QMessageBox::question(this, "Confirm Close Period",
        QString("Close fiscal period %1 to %2?\n\n"
                "This will zero out all revenue and expense accounts "
                "and transfer the net income to Retained Earnings.\n\n"
                "Journal entries with dates in this period will no longer be allowed.")
            .arg(start, end),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    if (m_db->closePeriod(start, end)) {
        QMessageBox::information(this, "Period Closed",
            "Fiscal period closed successfully.");
        accept();
    } else {
        QMessageBox::critical(this, "Error",
            "Failed to close period: " + m_db->lastError());
    }
}
