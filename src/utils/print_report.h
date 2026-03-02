#ifndef PRINT_REPORT_H
#define PRINT_REPORT_H

#include <QString>

class QTableWidget;
class QWidget;

// Prints a report table to PDF via a file save dialog.
// title is rendered as a header above the table.
// Returns true if the PDF was saved successfully.
bool printReportToPdf(QTableWidget *table, const QString &title,
                      QWidget *parent, const QString &defaultFileName = "report.pdf");

#endif // PRINT_REPORT_H
