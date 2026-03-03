# MoneyMan Feature Roadmap

Ordered feature list following depth-first approach: each feature is built completely before moving to the next. Order follows dependency chains.

## Constraints

- Zero per-usage cost — no paid APIs, cloud services, or subscriptions
- Single-user for now, keep door open for multi-user later
- Desktop-only (Qt6 Widgets, offline-first)
- Invoicing is "maybe eventually" — defer to later

## Current State

MoneyMan has: chart of accounts, journal entries with templates, trial balance, general ledger, income statement, balance sheet, fiscal period closing, audit log, dashboard, CSV import/export, PDF export, SQLCipher encryption, backup/restore, dark mode, in-app help guide.

## Roadmap

### 1. Date Range Filtering on All Reports

**Why first:** Every report shows all-time data. Smallest change with broadest impact — makes every existing report usable with real data. Lays groundwork for reconciliation and budgeting (both period-based).

**Scope:**
- Shared date range picker widget (QDateEdit from/to + presets: This Month, This Quarter, This Year, All Time)
- Wire into: Trial Balance, General Ledger, Income Statement, Balance Sheet, Journal List, Dashboard
- Reports query with date boundaries

**Depends on:** Nothing.

### 2. Bank Reconciliation with OFX Import

**Why second:** User downloads bank statements as CSV/OFX. This is the actual daily workflow — comparing bank records against the ledger. Without it, MoneyMan can't be the primary bookkeeping tool.

**Scope:**
- OFX file parser (OFX is XML-based, use Qt's XML module — no external library)
- Bank reconciliation view: import statement, show unmatched transactions side-by-side with ledger entries, click to match
- Reconciliation status on journal entries (unreconciled / reconciled / void)
- Reconciliation history in DB for audit trail

**Depends on:** Date range filtering (reconciliation is always for a specific period).

### 3. Recurring Transactions

**Why third:** Templates exist but have no scheduling. Extend with frequency and auto-posting.

**Scope:**
- `recurring_rules` table: template_id, frequency, next_due_date, end_date
- On startup or manual trigger: detect due entries, show confirmation dialog listing what will be posted
- Management UI: list recurring rules, pause/resume/delete

**Depends on:** Existing template system.

### 4. Budgeting & Variance

**Why fourth:** With reconciled, date-filtered data, budgets become meaningful.

**Scope:**
- `budgets` table: account_id, period_start, period_end, amount_cents
- Budget entry UI: spreadsheet-style grid (accounts as rows, months as columns)
- Variance column on Income Statement and Trial Balance (Budget | Actual | Variance)
- Dashboard widget showing over/under budget accounts

**Depends on:** Date range filtering.

### 5. Dashboard Charts

**Why fifth:** With date filtering, reconciliation, and budgets in place, there's meaningful data to visualize.

**Scope:**
- Cash flow over time (line chart)
- Expense breakdown by category (pie chart)
- Revenue vs. Expenses trend (bar chart)
- Budget vs. Actual (bar chart)
- Uses Qt Charts module (included with Qt6, no extra cost)

**Depends on:** Features 1-4 feed data into charts.

### 6. Invoicing & Accounts Receivable

**Why sixth:** "Maybe eventually." Larger module deferred until core bookkeeping workflow is solid.

**Scope:**
- `customers`, `invoices`, `invoice_lines` tables
- Invoice creation form with PDF generation (reuse print_report infrastructure)
- Payment recording (auto-generates journal entries)
- AR Aging report (0-30, 31-60, 61-90, 90+ days)

**Depends on:** Solid core bookkeeping (features 1-4).

### 7. Document Attachments

**Why last:** Nice-to-have for audit trail.

**Scope:**
- `document_attachments` table referencing journal entries, storing file paths (not BLOBs)
- File picker on journal entry dialog, thumbnail/link on General Ledger
- Managed directory alongside database

**Depends on:** Nothing technically, lowest priority.

## Research Sources

Based on competitive analysis of: GnuCash, MoneyManager Ex, HomeBank, Manager.io, Wave, ZipBooks, Akaunting, ERPNext.
