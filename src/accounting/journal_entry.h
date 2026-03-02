#ifndef JOURNAL_ENTRY_H
#define JOURNAL_ENTRY_H

#include <QString>
#include <vector>
#include <cstdint>

struct JournalLine {
    int64_t accountId = 0;
    int64_t debitCents = 0;
    int64_t creditCents = 0;
};

// Validates that a set of journal lines is balanced (debits == credits)
// and has at least 2 lines with non-zero totals.
// Returns empty string on success, error message on failure.
QString validateJournalLines(const std::vector<JournalLine> &lines);

#endif // JOURNAL_ENTRY_H
