#include "print_report.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QDateTime>

bool printReportToPdf(QTableWidget *table, const QString &title,
                      QWidget *parent, const QString &defaultFileName)
{
    QString path = QFileDialog::getSaveFileName(parent, "Export to PDF",
        defaultFileName, "PDF Files (*.pdf)");
    if (path.isEmpty())
        return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageOrientation(QPageLayout::Landscape);

    // Build HTML table from QTableWidget
    QString html;
    html += "<html><head><style>"
            "body { font-family: sans-serif; }"
            "h1 { font-size: 18pt; margin-bottom: 4px; }"
            ".date { font-size: 9pt; color: #666; margin-bottom: 12px; }"
            "table { border-collapse: collapse; width: 100%; }"
            "th { background: #333; color: white; padding: 6px 8px; text-align: left; font-size: 9pt; }"
            "td { padding: 4px 8px; border-bottom: 1px solid #ddd; font-size: 9pt; }"
            "tr:nth-child(even) { background: #f5f5f5; }"
            "</style></head><body>";
    html += "<h1>" + title.toHtmlEscaped() + "</h1>";
    html += "<div class='date'>Generated: "
          + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
          + "</div>";
    html += "<table><thead><tr>";

    int cols = table->columnCount();
    int rows = table->rowCount();

    for (int c = 0; c < cols; ++c) {
        auto *headerItem = table->horizontalHeaderItem(c);
        html += "<th>" + (headerItem ? headerItem->text().toHtmlEscaped() : QString()) + "</th>";
    }
    html += "</tr></thead><tbody>";

    for (int r = 0; r < rows; ++r) {
        html += "<tr>";
        for (int c = 0; c < cols; ++c) {
            auto *item = table->item(r, c);
            QString text = item ? item->text() : QString();
            html += "<td>" + text.toHtmlEscaped() + "</td>";
        }
        html += "</tr>";
    }

    html += "</tbody></table></body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    return true;
}
