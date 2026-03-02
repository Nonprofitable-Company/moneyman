#include "csv_import.h"

#include <QFile>
#include <QTextStream>

static QStringList parseCsvLine(const QString &line)
{
    QStringList fields;
    QString field;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        QChar c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i; // skip escaped quote
                } else {
                    inQuotes = false;
                }
            } else {
                field += c;
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ',') {
                fields.append(field.trimmed());
                field.clear();
            } else {
                field += c;
            }
        }
    }
    fields.append(field.trimmed());
    return fields;
}

std::vector<QStringList> parseCsvFile(const QString &filePath, QString &errorOut)
{
    std::vector<QStringList> rows;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorOut = "Cannot open file: " + file.errorString();
        return rows;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty())
            continue;
        rows.push_back(parseCsvLine(line));
    }

    return rows;
}
