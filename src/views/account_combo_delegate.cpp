#include "account_combo_delegate.h"
#include <QComboBox>

AccountComboDelegate::AccountComboDelegate(Database *db, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_db(db)
{
}

QWidget* AccountComboDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    auto *combo = new QComboBox(parent);
    combo->addItem("-- Select Account --", static_cast<qlonglong>(0));
    auto accounts = m_db->allAccounts();
    for (const auto &acct : accounts) {
        QString label = QString("%1 — %2").arg(acct.code).arg(acct.name);
        combo->addItem(label, static_cast<qlonglong>(acct.id));
    }
    return combo;
}

void AccountComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox*>(editor);
    if (!combo) return;

    qlonglong accountId = index.data(Qt::EditRole).toLongLong();
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i).toLongLong() == accountId) {
            combo->setCurrentIndex(i);
            return;
        }
    }
    combo->setCurrentIndex(0);
}

void AccountComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox*>(editor);
    if (!combo) return;

    qlonglong accountId = combo->currentData().toLongLong();
    model->setData(index, accountId, Qt::EditRole);
}

QString AccountComboDelegate::displayText(const QVariant &value, const QLocale & /*locale*/) const
{
    int64_t accountId = value.toLongLong();
    if (accountId <= 0) return "-- Select Account --";

    AccountRow acct = m_db->accountById(accountId);
    if (acct.id == 0) return "Unknown";
    return QString("%1 — %2").arg(acct.code).arg(acct.name);
}
