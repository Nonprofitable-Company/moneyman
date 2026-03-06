#ifndef TAX_CATEGORIES_H
#define TAX_CATEGORIES_H

#include <QStringList>

QStringList standardTaxCategories();

QString suggestTaxCategory(int code, const QString &name, const QString &type);

#endif // TAX_CATEGORIES_H
