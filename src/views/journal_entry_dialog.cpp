#include "journal_entry_dialog.h"
#include "account_combo_delegate.h"
#include "models/journal_line_model.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDateEdit>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDate>

JournalEntryDialog::JournalEntryDialog(Database *db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
    , m_lineModel(new JournalLineModel(this))
{
    setWindowTitle("New Journal Entry");
    setMinimumSize(650, 450);

    // Header form: date + description
    auto *formLayout = new QFormLayout;

    m_dateEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    formLayout->addRow("Date:", m_dateEdit);

    m_descriptionEdit = new QLineEdit(this);
    m_descriptionEdit->setPlaceholderText("e.g., Sale to customer, Rent payment");
    formLayout->addRow("Description:", m_descriptionEdit);

    // Lines table
    m_lineTable = new QTableView(this);
    m_lineTable->setModel(m_lineModel);
    m_lineTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_lineTable->setAlternatingRowColors(true);
    m_lineTable->horizontalHeader()->setStretchLastSection(true);
    m_lineTable->verticalHeader()->hide();
    m_lineTable->setColumnWidth(JournalLineModel::ColAccount, 280);
    m_lineTable->setColumnWidth(JournalLineModel::ColDebit, 120);
    m_lineTable->setColumnWidth(JournalLineModel::ColCredit, 120);

    // Account column uses a combo delegate
    auto *acctDelegate = new AccountComboDelegate(m_db, this);
    m_lineTable->setItemDelegateForColumn(JournalLineModel::ColAccount, acctDelegate);

    // Line add/remove buttons
    auto *lineButtons = new QHBoxLayout;
    auto *addLineBtn = new QPushButton("Add Line", this);
    auto *removeLineBtn = new QPushButton("Remove Line", this);
    lineButtons->addWidget(addLineBtn);
    lineButtons->addWidget(removeLineBtn);
    lineButtons->addStretch();

    connect(addLineBtn, &QPushButton::clicked, this, &JournalEntryDialog::onAddLine);
    connect(removeLineBtn, &QPushButton::clicked, this, &JournalEntryDialog::onRemoveLine);

    // Totals row
    auto *totalsLayout = new QHBoxLayout;
    totalsLayout->addStretch();
    totalsLayout->addWidget(new QLabel("Totals:", this));
    m_debitTotalLabel = new QLabel("0.00", this);
    m_creditTotalLabel = new QLabel("0.00", this);
    m_debitTotalLabel->setMinimumWidth(100);
    m_creditTotalLabel->setMinimumWidth(100);
    m_debitTotalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_creditTotalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    auto boldFont = m_debitTotalLabel->font();
    boldFont.setBold(true);
    m_debitTotalLabel->setFont(boldFont);
    m_creditTotalLabel->setFont(boldFont);
    totalsLayout->addWidget(new QLabel("Debit:", this));
    totalsLayout->addWidget(m_debitTotalLabel);
    totalsLayout->addWidget(new QLabel("Credit:", this));
    totalsLayout->addWidget(m_creditTotalLabel);

    // Balance status
    m_balanceStatusLabel = new QLabel(this);
    m_balanceStatusLabel->setAlignment(Qt::AlignCenter);

    // Post / Cancel buttons
    auto *buttonLayout = new QHBoxLayout;
    m_postButton = new QPushButton("Post Entry", this);
    m_postButton->setEnabled(false);
    auto *cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_postButton);
    buttonLayout->addWidget(cancelButton);

    connect(m_postButton, &QPushButton::clicked, this, &JournalEntryDialog::onPost);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_lineModel, &JournalLineModel::totalsChanged, this, &JournalEntryDialog::onTotalsChanged);

    // Main layout
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_lineTable, 1);
    mainLayout->addLayout(lineButtons);
    mainLayout->addLayout(totalsLayout);
    mainLayout->addWidget(m_balanceStatusLabel);
    mainLayout->addLayout(buttonLayout);

    onTotalsChanged();
}

void JournalEntryDialog::onAddLine()
{
    m_lineModel->addLine();
}

void JournalEntryDialog::onRemoveLine()
{
    auto selection = m_lineTable->selectionModel()->selectedRows();
    if (!selection.isEmpty()) {
        m_lineModel->removeLine(selection.first().row());
    }
}

void JournalEntryDialog::onTotalsChanged()
{
    double debits = static_cast<double>(m_lineModel->totalDebitCents()) / 100.0;
    double credits = static_cast<double>(m_lineModel->totalCreditCents()) / 100.0;

    m_debitTotalLabel->setText(QString::number(debits, 'f', 2));
    m_creditTotalLabel->setText(QString::number(credits, 'f', 2));

    bool balanced = m_lineModel->isBalanced();
    bool enoughLines = m_lineModel->validLineCount() >= 2;

    if (balanced && enoughLines) {
        m_balanceStatusLabel->setText("Balanced");
        m_balanceStatusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else if (!enoughLines) {
        m_balanceStatusLabel->setText("Need at least 2 lines with accounts and amounts");
        m_balanceStatusLabel->setStyleSheet("color: gray;");
    } else {
        double diff = debits - credits;
        m_balanceStatusLabel->setText(QString("Out of balance by %1").arg(
            QString::number(qAbs(diff), 'f', 2)));
        m_balanceStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    }

    m_postButton->setEnabled(balanced && enoughLines);
}

void JournalEntryDialog::onPost()
{
    QString desc = m_descriptionEdit->text().trimmed();
    if (desc.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Description cannot be empty.");
        return;
    }

    QString date = m_dateEdit->date().toString("yyyy-MM-dd");

    std::vector<JournalLineRow> dbLines;
    for (const auto &line : m_lineModel->lines()) {
        if (line.accountId <= 0 || (line.debitCents == 0 && line.creditCents == 0))
            continue;
        JournalLineRow row;
        row.accountId = line.accountId;
        row.debitCents = line.debitCents;
        row.creditCents = line.creditCents;
        dbLines.push_back(row);
    }

    if (!m_db->postJournalEntry(date, desc, dbLines)) {
        QMessageBox::warning(this, "Posting Error",
            "Failed to post journal entry: " + m_db->lastError());
        return;
    }

    accept();
}
