# MoneyMan

Double-entry bookkeeping desktop application for The Nonprofitable Company.

Built with C++17, Qt6 Widgets, and SQLCipher-encrypted SQLite.

## Features

- **Chart of Accounts** — GAAP-standard five-category structure (Assets, Liabilities, Equity, Revenue, Expenses) with customizable account codes
- **Journal Entries** — Double-entry transaction posting with enforced debit/credit balance, reusable templates
- **Reports** — Trial Balance, General Ledger, Income Statement, Balance Sheet
- **Fiscal Periods** — Period closing with automatic closing entries to Retained Earnings
- **Audit Log** — Append-only record of all actions (no deletions)
- **Dashboard** — Overview of financial position with recent activity
- **Import/Export** — CSV import for chart of accounts, CSV and PDF export for all reports
- **Encryption** — AES-256 database encryption via SQLCipher with passphrase management
- **Backup & Restore** — Full database backup and restore
- **In-App Help** — Built-in user guide covering GAAP fundamentals and app usage
- **Dark Mode** — Toggle via Window menu
- **Keyboard Shortcuts** — Ctrl+J (new entry), Ctrl+R (refresh), Ctrl+1–7 (switch tabs), F1 (help)

## Requirements

- C++17 compiler (GCC, Clang)
- CMake 3.20+
- Qt6 (Core, Widgets, Sql, PrintSupport)
- SQLCipher
- Catch2 v3 (for tests)

### Installing dependencies

**Arch Linux:**

```bash
pacman -S qt6-base cmake sqlcipher catch2
```

**macOS (Homebrew):**

```bash
brew install qt@6 cmake sqlcipher catch2
```

**Ubuntu/Debian:**

```bash
apt install qt6-base-dev libsqlcipher-dev cmake catch2
```

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Test

```bash
cd build && ctest --output-on-failure
```

## Run

```bash
./build/moneyman
```

You will be prompted for an encryption passphrase on first launch. This passphrase is required every time you open the application.

## Project Structure

```
src/
├── main.cpp
├── app/              # Application setup, main window
├── models/           # Qt models + business logic
├── views/            # Qt widgets / UI
├── db/               # Database access layer (SQLCipher)
├── accounting/       # Core accounting engine (double-entry logic)
└── utils/            # CSV/PDF export, CSV import
tests/
├── test_accounting/  # Accounting engine tests
└── test_db/          # Database tests
docs/
└── plans/            # Feature design documents
```

## Accounting Rules

- Every journal entry must have at least one debit and one credit line
- Total debits must equal total credits in every entry
- Assets and Expenses increase with debits
- Liabilities, Equity, and Revenue increase with credits
- No entry may be deleted — only reversing entries
- All monetary values stored as integer cents (int64_t)

## License

Proprietary — The Nonprofitable Company.
