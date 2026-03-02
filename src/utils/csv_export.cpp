#include "csv_export.h"

#include <QTableWidget>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

static QString escapeCsvField(const QString &field)
{
    if (field.contains(',') || field.contains('"') || field.contains('\n')) {
        QString escaped = field;
        escaped.replace('"', "\"\"");
        return '"' + escaped + '"';
    }
    return field;
}

bool exportTableToCsv(QTableWidget *table, QWidget *parent,
                      const QString &defaultFileName)
{
    QString filePath = QFileDialog::getSaveFileName(
        parent, "Export to CSV", defaultFileName, "CSV Files (*.csv)");

    if (filePath.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(parent, "Export Error",
            "Could not open file for writing: " + file.errorString());
        return false;
    }

    QTextStream out(&file);

    // Header row
    QStringList headers;
    for (int col = 0; col < table->columnCount(); ++col) {
        auto *item = table->horizontalHeaderItem(col);
        headers << escapeCsvField(item ? item->text() : QString());
    }
    out << headers.join(',') << '\n';

    // Data rows
    for (int row = 0; row < table->rowCount(); ++row) {
        QStringList fields;
        for (int col = 0; col < table->columnCount(); ++col) {
            auto *item = table->item(row, col);
            fields << escapeCsvField(item ? item->text() : QString());
        }
        out << fields.join(',') << '\n';
    }

    file.close();
    return true;
}
