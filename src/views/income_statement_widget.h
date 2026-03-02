#ifndef INCOME_STATEMENT_WIDGET_H
#define INCOME_STATEMENT_WIDGET_H

#include <QWidget>

class QTableWidget;
class QLabel;
class QDateEdit;
class QCheckBox;
class Database;

class IncomeStatementWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IncomeStatementWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();
    void exportCsv();

private:
    Database *m_db;
    QTableWidget *m_table;
    QLabel *m_netIncomeLabel;
    QCheckBox *m_dateFilterCheck;
    QDateEdit *m_fromDate;
    QDateEdit *m_toDate;
};

#endif // INCOME_STATEMENT_WIDGET_H
