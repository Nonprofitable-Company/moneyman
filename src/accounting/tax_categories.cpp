#include "tax_categories.h"

QStringList standardTaxCategories()
{
    return {
        "Gross Receipts",
        "Returns & Allowances",
        "Other Income",
        "Advertising",
        "Car & Truck Expenses",
        "Contract Labor",
        "Depreciation",
        "Insurance",
        "Interest",
        "Legal & Professional",
        "Meals",
        "Office Expense",
        "Rent",
        "Repairs & Maintenance",
        "Supplies",
        "Taxes & Licenses",
        "Travel",
        "Utilities",
        "Wages",
        "Other Expenses",
        "Not Tax-Relevant",
    };
}

QString suggestTaxCategory(int code, const QString &name, const QString &type)
{
    Q_UNUSED(code)

    if (type == "equity")
        return QStringLiteral("Not Tax-Relevant");

    if (type == "revenue")
        return QStringLiteral("Gross Receipts");

    if (type == "expense") {
        QString lower = name.toLower();
        if (lower.contains("rent"))                              return QStringLiteral("Rent");
        if (lower.contains("utilit"))                            return QStringLiteral("Utilities");
        if (lower.contains("wage") || lower.contains("payroll")
            || lower.contains("salary") || lower.contains("salaries"))
                                                                 return QStringLiteral("Wages");
        if (lower.contains("insurance"))                         return QStringLiteral("Insurance");
        if (lower.contains("depreci") || lower.contains("amort"))return QStringLiteral("Depreciation");
        if (lower.contains("interest"))                          return QStringLiteral("Interest");
        if (lower.contains("adverti") || lower.contains("marketing"))
                                                                 return QStringLiteral("Advertising");
        if (lower.contains("legal") || lower.contains("professional")
            || lower.contains("accounting"))
                                                                 return QStringLiteral("Legal & Professional");
        if (lower.contains("office"))                            return QStringLiteral("Office Expense");
        if (lower.contains("suppli"))                            return QStringLiteral("Supplies");
        if (lower.contains("travel"))                            return QStringLiteral("Travel");
        if (lower.contains("meal") || lower.contains("food")
            || lower.contains("dining"))
                                                                 return QStringLiteral("Meals");
        if (lower.contains("repair") || lower.contains("maintenance"))
                                                                 return QStringLiteral("Repairs & Maintenance");
        if (lower.contains("tax") || lower.contains("license"))  return QStringLiteral("Taxes & Licenses");
        if (lower.contains("contract") || lower.contains("freelance"))
                                                                 return QStringLiteral("Contract Labor");
        if (lower.contains("car") || lower.contains("truck")
            || lower.contains("vehicle") || lower.contains("auto"))
                                                                 return QStringLiteral("Car & Truck Expenses");
        return QStringLiteral("Other Expenses");
    }

    // asset, liability — no default tax category
    return {};
}
