#ifndef TRIAL_BALANCE_WIDGET_H
#define TRIAL_BALANCE_WIDGET_H

#include <QWidget>

class QTableWidget;
class QLabel;
class Database;

class TrialBalanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrialBalanceWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    Database *m_db;
    QTableWidget *m_table;
    QLabel *m_debitTotalLabel;
    QLabel *m_creditTotalLabel;
    QLabel *m_statusLabel;
};

#endif // TRIAL_BALANCE_WIDGET_H
