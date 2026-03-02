#ifndef CSV_IMPORT_H
#define CSV_IMPORT_H

#include <QString>
#include <QStringList>
#include <vector>

// Parses a CSV file into rows of string fields.
// Handles quoted fields and embedded commas.
// Returns empty vector on error, with errorOut set.
std::vector<QStringList> parseCsvFile(const QString &filePath, QString &errorOut);

#endif // CSV_IMPORT_H
