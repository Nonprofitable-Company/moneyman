#include <catch2/catch_test_macros.hpp>
#include "accounting/account.h"

TEST_CASE("AccountType string conversion", "[account]")
{
    SECTION("to string") {
        REQUIRE(accountTypeToString(AccountType::Asset) == "asset");
        REQUIRE(accountTypeToString(AccountType::Liability) == "liability");
        REQUIRE(accountTypeToString(AccountType::Equity) == "equity");
        REQUIRE(accountTypeToString(AccountType::Revenue) == "revenue");
        REQUIRE(accountTypeToString(AccountType::Expense) == "expense");
    }

    SECTION("from string") {
        REQUIRE(accountTypeFromString("asset") == AccountType::Asset);
        REQUIRE(accountTypeFromString("liability") == AccountType::Liability);
        REQUIRE(accountTypeFromString("equity") == AccountType::Equity);
        REQUIRE(accountTypeFromString("revenue") == AccountType::Revenue);
        REQUIRE(accountTypeFromString("expense") == AccountType::Expense);
    }
}

TEST_CASE("Debit normal accounts", "[account]")
{
    REQUIRE(isDebitNormal(AccountType::Asset) == true);
    REQUIRE(isDebitNormal(AccountType::Expense) == true);
    REQUIRE(isDebitNormal(AccountType::Liability) == false);
    REQUIRE(isDebitNormal(AccountType::Equity) == false);
    REQUIRE(isDebitNormal(AccountType::Revenue) == false);
}
