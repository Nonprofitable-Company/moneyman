#include "journal_entry.h"

QString validateJournalLines(const std::vector<JournalLine> &lines)
{
    if (lines.size() < 2) {
        return "Journal entry must have at least 2 lines";
    }

    int64_t totalDebits = 0;
    int64_t totalCredits = 0;

    for (const auto &line : lines) {
        if (line.debitCents < 0 || line.creditCents < 0) {
            return "Amounts must not be negative";
        }
        if (line.debitCents == 0 && line.creditCents == 0) {
            return "Each line must have a debit or credit amount";
        }
        if (line.debitCents > 0 && line.creditCents > 0) {
            return "A line cannot have both debit and credit";
        }
        totalDebits += line.debitCents;
        totalCredits += line.creditCents;
    }

    if (totalDebits == 0) {
        return "Entry must have non-zero amounts";
    }

    if (totalDebits != totalCredits) {
        return "Debits must equal credits";
    }

    return {};
}
