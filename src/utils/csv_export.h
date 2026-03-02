#ifndef CSV_EXPORT_H
#define CSV_EXPORT_H

#include <QString>

class QTableWidget;
class QWidget;

// Exports a QTableWidget's contents to CSV via a file save dialog.
// Returns true if the file was saved successfully.
bool exportTableToCsv(QTableWidget *table, QWidget *parent,
                      const QString &defaultFileName = "export.csv");

#endif // CSV_EXPORT_H
