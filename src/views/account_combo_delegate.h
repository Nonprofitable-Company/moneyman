#ifndef ACCOUNT_COMBO_DELEGATE_H
#define ACCOUNT_COMBO_DELEGATE_H

#include <QStyledItemDelegate>
#include <vector>
#include "db/database.h"

class AccountComboDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit AccountComboDelegate(Database *db, QObject *parent = nullptr);

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    QString displayText(const QVariant &value, const QLocale &locale) const override;

private:
    Database *m_db;
};

#endif // ACCOUNT_COMBO_DELEGATE_H
