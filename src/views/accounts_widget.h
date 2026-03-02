#ifndef ACCOUNTS_WIDGET_H
#define ACCOUNTS_WIDGET_H

#include <QWidget>

class QTableView;
class QLineEdit;
class QSortFilterProxyModel;
class AccountModel;
class Database;

class AccountsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AccountsWidget(Database *db, QWidget *parent = nullptr);

    AccountModel* model() const { return m_model; }

private slots:
    void onAddAccount();
    void onFilterChanged(const QString &text);

private:
    Database *m_db;
    AccountModel *m_model;
    QSortFilterProxyModel *m_proxyModel;
    QTableView *m_tableView;
    QLineEdit *m_filterEdit;
};

#endif // ACCOUNTS_WIDGET_H
