#ifndef JOURNAL_LIST_WIDGET_H
#define JOURNAL_LIST_WIDGET_H

#include <QWidget>

class Database;
class QTableWidget;
class QLineEdit;

class JournalListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JournalListWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onFilterChanged(const QString &text);
    void onContextMenu(const QPoint &pos);
    void exportCsv();

private:
    Database *m_db;
    QTableWidget *m_table;
    QLineEdit *m_filterEdit;
};

#endif // JOURNAL_LIST_WIDGET_H
