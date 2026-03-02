#include "accounts_widget.h"
#include "add_account_dialog.h"
#include "models/account_model.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QToolBar>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QShortcut>
#include <QKeySequence>

AccountsWidget::AccountsWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_model(new AccountModel(db, this))
    , m_tableView(new QTableView(this))
{
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setShowGrid(true);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(AccountModel::ColName, QHeaderView::Stretch);
    m_tableView->verticalHeader()->hide();
    m_tableView->setColumnWidth(AccountModel::ColCode, 70);
    m_tableView->setColumnWidth(AccountModel::ColType, 90);
    m_tableView->setColumnWidth(AccountModel::ColBalance, 100);

    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(
        style->standardIcon(QStyle::SP_FileDialogNewFolder),
        "Add Account (Ctrl+A)", this, &AccountsWidget::onAddAccount);

    // Ctrl+A shortcut for Add Account
    auto *shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_A), this);
    connect(shortcut, &QShortcut::activated, this, &AccountsWidget::onAddAccount);

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
