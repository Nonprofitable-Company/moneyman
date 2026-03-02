#ifndef INCOME_STATEMENT_WIDGET_H
#define INCOME_STATEMENT_WIDGET_H

#include <QWidget>

class QTableWidget;
class QLabel;
class Database;

class IncomeStatementWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IncomeStatementWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    Database *m_db;
    QTableWidget *m_table;
    QLabel *m_netIncomeLabel;
};

#endif // INCOME_STATEMENT_WIDGET_H
