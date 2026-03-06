# MoneyMan

Double-entry bookkeeping desktop application for The Nonprofitable Company.

## Tech Stack

- **Language:** C++17 (or C++20 where supported)
- **Framework:** Qt6 (widgets-based, not QML)
- **Database:** SQLite with SQLCipher for encryption-at-rest
- **Build:** CMake
- **Testing:** Qt Test + Catch2
- **Platforms:** Linux, macOS, Windows
- **CI/CD:** GitHub Actions (build, test, release for all platforms)

## Architecture

- Offline-first — app must work with zero network connectivity
- SQLCipher-encrypted SQLite database for all financial data
- Model-View architecture using Qt's model/view framework
- Double-entry accounting: every transaction has balanced debits and credits

## Coding Standards

### Naming

- Classes: PascalCase (e.g., `JournalEntry`, `AccountModel`)
- Methods/functions: camelCase (e.g., `postTransaction()`)
- Member variables: m_ prefix (e.g., `m_balance`)
- Files: snake_case matching class name (e.g., `journal_entry.h`)

### Project Structure

```
src/
├── main.cpp
├── app/              # Application setup, main window
├── models/           # Qt models + business logic
├── views/            # Qt widgets / UI
├── db/               # Database access layer (SQLCipher)
├── accounting/       # Core accounting engine (double-entry logic)
└── utils/            # Shared utilities
tests/
├── test_accounting/  # Accounting engine tests
├── test_models/      # Model tests
└── test_db/          # Database tests
```

### Rules

- Every transaction MUST balance: total debits == total credits
- No raw SQL outside `src/db/` — all queries go through the DB layer
- All monetary values use integer cents (int64_t), never floating point
- UI and business logic must be separated — models are testable without UI

## Commands

### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```

### Test

```bash
cd build && ctest --output-on-failure
```

### Run

```bash
./build/moneyman
```

### Verification (MUST pass before considering work complete)

1. `cmake --build build` compiles without errors or warnings (-Werror)
2. `ctest --output-on-failure` — all tests pass
3. New accounting logic has corresponding test in tests/
4. Debits == Credits invariant enforced in all transaction code

## Double-Entry Accounting Rules

### Chart of Accounts (Standard Small Business)

- **Assets** (1000-1999): Bank, Accounts Receivable, Equipment
- **Liabilities** (2000-2999): Accounts Payable, Loans, Credit Cards
- **Equity** (3000-3999): Owner's Equity, Retained Earnings
- **Revenue** (4000-4999): Sales, Service Income
- **Expenses** (5000-5999): Rent, Utilities, Supplies, Payroll

### Core Invariants

- Every journal entry has 2+ lines
- Sum of debits MUST equal sum of credits in every entry
- Assets + Expenses increase with DEBITS
- Liabilities + Equity + Revenue increase with CREDITS
- No entry may be deleted — only reversing entries allowed
- All entries must have a date, description, and at least one debit and one credit line

### Key Entities

- **Account**: code, name, type (asset/liability/equity/revenue/expense), balance
- **JournalEntry**: date, description, lines[], posted flag
- **JournalLine**: account_id, debit_amount, credit_amount
- **FiscalPeriod**: start_date, end_date, closed flag

## Development Process (Ralph Loop)

### Priorities (in order)

1. Make it compile
2. Make tests pass
3. Implement the requested feature
4. Refactor only if needed

### Per-Iteration Checklist

- Read recent git log to understand what was done in previous iterations
- Check build status first — fix compilation errors before adding features
- Run tests — fix failures before adding new code
- Make ONE logical change per iteration, then commit
- Write a clear git commit message describing what changed

### Git Discipline

- Commit after each meaningful change
- Never commit code that doesn't compile
- Commit message format: `type: description` (feat, fix, refactor, test, docs)

### Branching & Worktrees

- Every new feature or GitHub issue gets its own branch and worktree
- Use `.worktrees/` directory for git worktrees (project-local, gitignored)
- Each branch maps to a PR — one branch per issue/feature

## Windows / Cross-Platform

### SQLCipher

- **Linux:** `sudo apt-get install libsqlcipher-dev` (PkgConfig)
- **macOS:** `brew install sqlcipher` (PkgConfig)
- **Windows:** `vcpkg install sqlcipher:x64-windows` (CMake `find_package`)

### Releases

Versioning: `MoneyMan 2`, `MoneyMan 3`, etc. Git tags: `v2`, `v3`, etc.

Push a tag to create a GitHub Release with artifacts for all platforms:
```bash
git tag v2 && git push origin v2
```
