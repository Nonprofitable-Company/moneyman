#include <catch2/catch_test_macros.hpp>
#include "accounting/tax_categories.h"

TEST_CASE("standardTaxCategories returns non-empty list", "[tax]")
{
    auto categories = standardTaxCategories();
    REQUIRE(!categories.isEmpty());
    REQUIRE(categories.contains("Gross Receipts"));
    REQUIRE(categories.contains("Rent"));
    REQUIRE(categories.contains("Not Tax-Relevant"));
}

TEST_CASE("suggestTaxCategory", "[tax]")
{
    SECTION("revenue suggests Gross Receipts") {
        REQUIRE(suggestTaxCategory(4000, "Sales Revenue", "revenue") == "Gross Receipts");
    }

    SECTION("equity suggests Not Tax-Relevant") {
        REQUIRE(suggestTaxCategory(3000, "Owner's Equity", "equity") == "Not Tax-Relevant");
    }

    SECTION("expense with rent keyword") {
        REQUIRE(suggestTaxCategory(5000, "Rent Expense", "expense") == "Rent");
    }

    SECTION("expense with utilities keyword") {
        REQUIRE(suggestTaxCategory(5100, "Utilities", "expense") == "Utilities");
    }

    SECTION("expense with wages keyword") {
        REQUIRE(suggestTaxCategory(5200, "Wages & Payroll", "expense") == "Wages");
    }

    SECTION("unknown expense defaults to Other Expenses") {
        REQUIRE(suggestTaxCategory(5900, "Miscellaneous", "expense") == "Other Expenses");
    }

    SECTION("asset returns empty") {
        REQUIRE(suggestTaxCategory(1000, "Cash", "asset").isEmpty());
    }

    SECTION("liability returns empty") {
        REQUIRE(suggestTaxCategory(2000, "Accounts Payable", "liability").isEmpty());
    }
}
