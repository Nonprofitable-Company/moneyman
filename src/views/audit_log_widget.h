#ifndef AUDIT_LOG_WIDGET_H
#define AUDIT_LOG_WIDGET_H

#include <QWidget>

class QTableWidget;
class Database;

class AuditLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AuditLogWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();
    void exportCsv();

private:
    Database *m_db;
    QTableWidget *m_table;
};

#endif // AUDIT_LOG_WIDGET_H
