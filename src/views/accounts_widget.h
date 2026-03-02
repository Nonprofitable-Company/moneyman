#ifndef ACCOUNTS_WIDGET_H
#define ACCOUNTS_WIDGET_H

#include <QWidget>

class QTableView;
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

private:
    Database *m_db;
    AccountModel *m_model;
    QTableView *m_tableView;
};

#endif // ACCOUNTS_WIDGET_H
