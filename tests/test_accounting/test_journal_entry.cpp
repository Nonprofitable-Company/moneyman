#include <catch2/catch_test_macros.hpp>
#include "accounting/journal_entry.h"

TEST_CASE("Journal entry validation", "[journal]")
{
    SECTION("valid balanced entry") {
        std::vector<JournalLine> lines = {
            {1, 10000, 0},  // debit $100
            {2, 0, 10000},  // credit $100
        };
        REQUIRE(validateJournalLines(lines).isEmpty());
    }

    SECTION("multi-line balanced entry") {
        std::vector<JournalLine> lines = {
            {1, 5000, 0},
            {2, 5000, 0},
            {3, 0, 10000},
        };
        REQUIRE(validateJournalLines(lines).isEmpty());
    }

    SECTION("rejects single line") {
        std::vector<JournalLine> lines = {
            {1, 10000, 0},
        };
        REQUIRE(validateJournalLines(lines) == "Journal entry must have at least 2 lines");
    }

    SECTION("rejects empty") {
        std::vector<JournalLine> lines;
        REQUIRE(validateJournalLines(lines) == "Journal entry must have at least 2 lines");
    }

    SECTION("rejects unbalanced entry") {
        std::vector<JournalLine> lines = {
            {1, 10000, 0},
            {2, 0, 5000},
        };
        REQUIRE(validateJournalLines(lines) == "Debits must equal credits");
    }

    SECTION("rejects zero amounts") {
        std::vector<JournalLine> lines = {
            {1, 0, 0},
            {2, 0, 0},
        };
        REQUIRE(validateJournalLines(lines) == "Each line must have a debit or credit amount");
    }

    SECTION("rejects negative amounts") {
        std::vector<JournalLine> lines = {
            {1, -100, 0},
            {2, 0, -100},
        };
        REQUIRE(validateJournalLines(lines) == "Amounts must not be negative");
    }

    SECTION("rejects line with both debit and credit") {
        std::vector<JournalLine> lines = {
            {1, 10000, 5000},
            {2, 0, 5000},
        };
        REQUIRE(validateJournalLines(lines) == "A line cannot have both debit and credit");
    }
}
