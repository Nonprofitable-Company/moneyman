#ifndef ACCOUNT_MODEL_H
#define ACCOUNT_MODEL_H

#include <QAbstractTableModel>
#include <vector>
#include "db/database.h"

class AccountModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column { ColCode = 0, ColName, ColType, ColCurrency, ColTaxCategory, ColBalance, ColCount };

    explicit AccountModel(Database *db, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void refresh();
    const AccountRow* accountAt(int row) const;

private:
    Database *m_db;
    std::vector<AccountRow> m_accounts;
};

#endif // ACCOUNT_MODEL_H
