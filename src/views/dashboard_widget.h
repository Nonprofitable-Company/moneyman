#ifndef DASHBOARD_WIDGET_H
#define DASHBOARD_WIDGET_H

#include <QWidget>

class Database;
class QLabel;
class QTableWidget;
class QFrame;

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    struct MetricCard {
        QFrame *frame;
        QLabel *title;
        QLabel *value;
    };

    MetricCard makeMetricCard(const QString &title, bool accent = false);

    Database *m_db;
    MetricCard m_totalAssets;
    MetricCard m_totalLiabilities;
    MetricCard m_totalEquity;
    MetricCard m_totalRevenue;
    MetricCard m_totalExpenses;
    MetricCard m_netIncome;
    MetricCard m_accountCount;
    MetricCard m_entryCount;
    MetricCard m_balanceStatus;
    QTableWidget *m_recentEntries;
    QTableWidget *m_recentAudit;
};

#endif // DASHBOARD_WIDGET_H
