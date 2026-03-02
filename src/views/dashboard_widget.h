#ifndef DASHBOARD_WIDGET_H
#define DASHBOARD_WIDGET_H

#include <QWidget>

class Database;
class QLabel;
class QTableWidget;

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    QLabel *makeMetricCard(const QString &title, QWidget *parent);

    Database *m_db;
    QLabel *m_totalAssets;
    QLabel *m_totalLiabilities;
    QLabel *m_totalEquity;
    QLabel *m_totalRevenue;
    QLabel *m_totalExpenses;
    QLabel *m_netIncome;
    QLabel *m_accountCount;
    QLabel *m_entryCount;
    QLabel *m_balanceStatus;
    QTableWidget *m_recentEntries;
    QTableWidget *m_recentAudit;
};

#endif // DASHBOARD_WIDGET_H
