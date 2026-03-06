#include "account_model.h"

AccountModel::AccountModel(Database *db, QObject *parent)
    : QAbstractTableModel(parent)
    , m_db(db)
{
    refresh();
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(m_accounts.size());
}

int AccountModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ColCount;
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_accounts.size()))
        return {};

    const auto &acct = m_accounts[static_cast<size_t>(index.row())];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColCode:    return acct.code;
        case ColName:    return acct.name;
        case ColType:    return acct.type;
        case ColCurrency: return acct.currency;
        case ColTaxCategory: return acct.taxCategory;
        case ColBalance: {
            // Format cents as dollars with 2 decimal places
            double dollars = static_cast<double>(acct.balanceCents) / 100.0;
            return QString::number(dollars, 'f', 2);
        }
        }
    }

    if (role == Qt::TextAlignmentRole && index.column() == ColBalance) {
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    }

    return {};
}

QVariant AccountModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case ColCode:    return "Code";
    case ColName:    return "Name";
    case ColType:    return "Type";
    case ColCurrency: return "Ccy";
    case ColTaxCategory: return "Tax Category";
    case ColBalance: return "Balance";
    }
    return {};
}

void AccountModel::refresh()
{
    beginResetModel();
    m_accounts = m_db->allAccounts();
    endResetModel();
}

const AccountRow* AccountModel::accountAt(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_accounts.size()))
        return nullptr;
    return &m_accounts[static_cast<size_t>(row)];
}
