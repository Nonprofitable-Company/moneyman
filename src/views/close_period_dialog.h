#ifndef CLOSE_PERIOD_DIALOG_H
#define CLOSE_PERIOD_DIALOG_H

#include <QDialog>

class QDateEdit;
class QLabel;
class Database;

class ClosePeriodDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClosePeriodDialog(Database *db, QWidget *parent = nullptr);

private slots:
    void onClose();

private:
    Database *m_db;
    QDateEdit *m_startDate;
    QDateEdit *m_endDate;
    QLabel *m_summaryLabel;

    void updateSummary();
};

#endif // CLOSE_PERIOD_DIALOG_H
