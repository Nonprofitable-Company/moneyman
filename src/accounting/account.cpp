#include "account.h"

QString accountTypeToString(AccountType type)
{
    switch (type) {
    case AccountType::Asset:     return "asset";
    case AccountType::Liability: return "liability";
    case AccountType::Equity:    return "equity";
    case AccountType::Revenue:   return "revenue";
    case AccountType::Expense:   return "expense";
    }
    return "asset";
}

AccountType accountTypeFromString(const QString &str)
{
    if (str == "liability") return AccountType::Liability;
    if (str == "equity")    return AccountType::Equity;
    if (str == "revenue")   return AccountType::Revenue;
    if (str == "expense")   return AccountType::Expense;
    return AccountType::Asset;
}

bool isDebitNormal(AccountType type)
{
    return type == AccountType::Asset || type == AccountType::Expense;
}
