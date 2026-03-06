#ifndef TAX_SUMMARY_WIDGET_H
#define TAX_SUMMARY_WIDGET_H

#include <QWidget>

class QTableWidget;
class QLabel;
class QDateEdit;
class Database;

class TaxSummaryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaxSummaryWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();
    void exportCsv();

private:
    Database *m_db;
    QTableWidget *m_table;
    QLabel *m_netIncomeLabel;
    QDateEdit *m_fromDate;
    QDateEdit *m_toDate;
};

#endif // TAX_SUMMARY_WIDGET_H
