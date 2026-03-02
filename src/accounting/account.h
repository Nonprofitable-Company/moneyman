#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QString>
#include <cstdint>

enum class AccountType {
    Asset,
    Liability,
    Equity,
    Revenue,
    Expense
};

QString accountTypeToString(AccountType type);
AccountType accountTypeFromString(const QString &str);

// Returns true if the account type increases with debits (Asset, Expense)
bool isDebitNormal(AccountType type);

#endif // ACCOUNT_H
