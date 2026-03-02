#ifndef BALANCE_SHEET_WIDGET_H
#define BALANCE_SHEET_WIDGET_H

#include <QWidget>

class QTableWidget;
class QLabel;
class Database;

class BalanceSheetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BalanceSheetWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    Database *m_db;
    QTableWidget *m_table;
    QLabel *m_statusLabel;
};

#endif // BALANCE_SHEET_WIDGET_H
