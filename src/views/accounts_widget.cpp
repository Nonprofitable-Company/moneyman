#include "accounts_widget.h"
#include "add_account_dialog.h"
#include "models/account_model.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QToolBar>
#include <QHeaderView>
#include <QMessageBox>

AccountsWidget::AccountsWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_model(new AccountModel(db, this))
    , m_tableView(new QTableView(this))
{
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->hide();
    m_tableView->setColumnWidth(AccountModel::ColCode, 80);
    m_tableView->setColumnWidth(AccountModel::ColName, 250);
    m_tableView->setColumnWidth(AccountModel::ColType, 100);

    auto *toolbar = new QToolBar(this);
    toolbar->addAction("Add Account", this, &AccountsWidget::onAddAccount);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(m_tableView);
}

void AccountsWidget::onAddAccount()
{
    AddAccountDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString name = dialog.accountName();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Account name cannot be empty.");
        return;
    }

    if (!m_db->createAccount(dialog.accountCode(), name, dialog.accountType())) {
        QMessageBox::warning(this, "Error",
            "Failed to create account: " + m_db->lastError());
        return;
    }

    m_model->refresh();
}
