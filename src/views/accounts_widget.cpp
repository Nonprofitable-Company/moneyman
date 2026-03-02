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
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QMenu>

AccountsWidget::AccountsWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_model(new AccountModel(db, this))
    , m_proxyModel(new QSortFilterProxyModel(this))
    , m_tableView(new QTableView(this))
    , m_filterEdit(new QLineEdit(this))
{
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1);

    m_tableView->setModel(m_proxyModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setShowGrid(true);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(AccountModel::ColName, QHeaderView::Stretch);
    m_tableView->verticalHeader()->hide();
    m_tableView->setColumnWidth(AccountModel::ColCode, 70);
    m_tableView->setColumnWidth(AccountModel::ColType, 90);
    m_tableView->setColumnWidth(AccountModel::ColCurrency, 50);
    m_tableView->setColumnWidth(AccountModel::ColBalance, 100);
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableView, &QTableView::customContextMenuRequested,
            this, &AccountsWidget::onContextMenu);

    m_filterEdit->setPlaceholderText("Filter by name or code...");
    m_filterEdit->setClearButtonEnabled(true);
    connect(m_filterEdit, &QLineEdit::textChanged, this, &AccountsWidget::onFilterChanged);

    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(
        style->standardIcon(QStyle::SP_FileDialogNewFolder),
        "Add Account (Ctrl+A)", this, &AccountsWidget::onAddAccount);

    auto *shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_A), this);
    connect(shortcut, &QShortcut::activated, this, &AccountsWidget::onAddAccount);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(m_filterEdit);
    layout->addWidget(m_tableView);
}

void AccountsWidget::onFilterChanged(const QString &text)
{
    m_proxyModel->setFilterFixedString(text);
}

void AccountsWidget::onContextMenu(const QPoint &pos)
{
    QModelIndex proxyIndex = m_tableView->indexAt(pos);
    if (!proxyIndex.isValid()) return;

    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    const AccountRow *acct = m_model->accountAt(sourceIndex.row());
    if (!acct) return;

    QMenu menu(this);
    menu.addAction("Edit Account...", this, &AccountsWidget::onEditAccount);
    menu.addAction("Delete Account...", this, [this]() {
        QModelIndex proxyIndex = m_tableView->currentIndex();
        if (!proxyIndex.isValid()) return;
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        const AccountRow *acct = m_model->accountAt(sourceIndex.row());
        if (!acct) return;

        auto reply = QMessageBox::question(this, "Delete Account",
            QString("Delete account %1 — %2?\n\nThis cannot be undone.")
                .arg(acct->code).arg(acct->name));
        if (reply != QMessageBox::Yes) return;

        if (!m_db->deleteAccount(acct->id)) {
            QMessageBox::warning(this, "Cannot Delete", m_db->lastError());
            return;
        }
        m_model->refresh();
    });
    menu.exec(m_tableView->viewport()->mapToGlobal(pos));
}

void AccountsWidget::onEditAccount()
{
    QModelIndex proxyIndex = m_tableView->currentIndex();
    if (!proxyIndex.isValid()) return;

    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    const AccountRow *acct = m_model->accountAt(sourceIndex.row());
    if (!acct) return;

    AddAccountDialog dialog(this);
    dialog.setEditMode(acct->code, acct->name, acct->type);

    if (dialog.exec() != QDialog::Accepted) return;

    QString name = dialog.accountName();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Account name cannot be empty.");
        return;
    }

    if (!m_db->updateAccount(acct->id, name, dialog.accountType())) {
        QMessageBox::warning(this, "Error",
            "Failed to update account: " + m_db->lastError());
        return;
    }

    m_model->refresh();
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

    if (!m_db->createAccount(dialog.accountCode(), name, dialog.accountType(), dialog.accountCurrency())) {
        QMessageBox::warning(this, "Error",
            "Failed to create account: " + m_db->lastError());
        return;
    }

    m_model->refresh();
}
