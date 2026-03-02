#ifndef IMPORT_CSV_DIALOG_H
#define IMPORT_CSV_DIALOG_H

#include <QDialog>

class Database;
class QTableWidget;
class QPushButton;
class QLabel;

class ImportCsvDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportCsvDialog(Database *db, QWidget *parent = nullptr);

    int importedCount() const { return m_importedCount; }

private slots:
    void onBrowse();
    void onImport();

private:
    Database *m_db;
    QTableWidget *m_preview;
    QPushButton *m_importBtn;
    QLabel *m_statusLabel;
    QString m_filePath;
    int m_importedCount = 0;
};

#endif // IMPORT_CSV_DIALOG_H
