#include "help_browser_dialog.h"

#include <QListWidget>
#include <QTextBrowser>
#include <QSplitter>
#include <QVBoxLayout>

HelpBrowserDialog::HelpBrowserDialog(QWidget *parent)
    : QDialog(parent)
    , m_tocList(nullptr)
    , m_browser(nullptr)
{
    setupUi();
    populateToc();
}

void HelpBrowserDialog::setupUi()
{
    setWindowTitle("MoneyMan User Guide");
    resize(900, 650);

    m_tocList = new QListWidget;
    m_tocList->setMaximumWidth(250);
    m_tocList->setMinimumWidth(180);

    m_browser = new QTextBrowser;
    m_browser->setOpenExternalLinks(false);
    m_browser->setHtml(helpContent());

    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(m_tocList);
    splitter->addWidget(m_browser);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({250, 650});

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(splitter);
}

void HelpBrowserDialog::populateToc()
{
    struct TocEntry {
        QString label;
        QString anchor;
    };

    const QList<TocEntry> entries = {
        {"Getting Started",       "getting-started"},
        {"Chart of Accounts",     "chart-of-accounts"},
        {"Journal Entries",       "journal-entries"},
        {"The Accounting Equation", "accounting-equation"},
        {"Trial Balance",         "trial-balance"},
        {"General Ledger",        "general-ledger"},
        {"Income Statement",      "income-statement"},
        {"Balance Sheet",         "balance-sheet"},
        {"Fiscal Periods",        "fiscal-periods"},
        {"Data Management",       "data-management"},
    };

    for (const auto &entry : entries) {
        auto *item = new QListWidgetItem(entry.label);
        item->setData(Qt::UserRole, entry.anchor);
        m_tocList->addItem(item);
    }

    connect(m_tocList, &QListWidget::currentItemChanged, this,
        [this](QListWidgetItem *current, QListWidgetItem *) {
            if (!current) return;
            QString anchor = current->data(Qt::UserRole).toString();
            m_browser->scrollToAnchor(anchor);
        });
}

QString HelpBrowserDialog::helpContent() const
{
    return QStringLiteral(R"(
<html>
<head>
<style>
body { font-family: sans-serif; margin: 16px; line-height: 1.5; }
h1 { color: #2c3e50; border-bottom: 2px solid #2c3e50; padding-bottom: 6px; }
h2 { color: #34495e; margin-top: 32px; border-bottom: 1px solid #bdc3c7; padding-bottom: 4px; }
h3 { color: #7f8c8d; }
code { background: #ecf0f1; padding: 2px 5px; border-radius: 3px; }
table { border-collapse: collapse; margin: 10px 0; }
th, td { border: 1px solid #bdc3c7; padding: 6px 12px; text-align: left; }
th { background: #ecf0f1; }
.tip { background: #eaf6ff; border-left: 4px solid #3498db; padding: 8px 12px; margin: 10px 0; }
.warn { background: #fff8e1; border-left: 4px solid #f39c12; padding: 8px 12px; margin: 10px 0; }
.key { background: #ecf0f1; border: 1px solid #bdc3c7; border-radius: 3px; padding: 1px 6px; font-size: 0.9em; }
</style>
</head>
<body>

<h1>MoneyMan User Guide</h1>
<p>A double-entry bookkeeping application for The Nonprofitable Company.</p>

<!-- ═══════════════════════════════════════════ -->
<a name="getting-started"></a>
<h2>1. Getting Started</h2>

<h3>What is Double-Entry Bookkeeping?</h3>
<p>Double-entry bookkeeping is the foundation of modern accounting. Every financial
transaction is recorded in <b>at least two accounts</b>: one account is debited and
another is credited. The total debits must always equal the total credits.</p>

<p>This system, formalized by Luca Pacioli in 1494, provides a built-in error-detection
mechanism. If your books don't balance, something was recorded incorrectly.</p>

<h3>How MoneyMan Works</h3>
<p>MoneyMan implements double-entry bookkeeping with these core concepts:</p>
<ul>
  <li><b>Chart of Accounts</b> &mdash; the list of all accounts in your organization</li>
  <li><b>Journal Entries</b> &mdash; the record of every transaction (debits and credits)</li>
  <li><b>Reports</b> &mdash; Trial Balance, General Ledger, Income Statement, Balance Sheet</li>
  <li><b>Fiscal Periods</b> &mdash; time boundaries for financial reporting</li>
</ul>

<div class="tip"><b>Keyboard shortcuts:</b>
<span class="key">Ctrl+J</span> New Journal Entry &bull;
<span class="key">Ctrl+R</span> Refresh Reports &bull;
<span class="key">Ctrl+1</span>&ndash;<span class="key">Ctrl+7</span> Switch report tabs &bull;
<span class="key">F1</span> This help guide</div>

<!-- ═══════════════════════════════════════════ -->
<a name="chart-of-accounts"></a>
<h2>2. Chart of Accounts</h2>

<h3>GAAP Account Categories</h3>
<p>Under Generally Accepted Accounting Principles (GAAP), accounts fall into five categories:</p>

<table>
<tr><th>Category</th><th>Code Range</th><th>Normal Balance</th><th>Examples</th></tr>
<tr><td>Assets</td><td>1000&ndash;1999</td><td>Debit</td><td>Cash, Equipment, Accounts Receivable</td></tr>
<tr><td>Liabilities</td><td>2000&ndash;2999</td><td>Credit</td><td>Accounts Payable, Loans, Credit Cards</td></tr>
<tr><td>Equity</td><td>3000&ndash;3999</td><td>Credit</td><td>Owner's Equity, Retained Earnings</td></tr>
<tr><td>Revenue</td><td>4000&ndash;4999</td><td>Credit</td><td>Sales, Service Income</td></tr>
<tr><td>Expenses</td><td>5000&ndash;5999</td><td>Debit</td><td>Rent, Utilities, Payroll</td></tr>
</table>

<p>The <b>normal balance</b> indicates which side (debit or credit) increases the account.
Assets and Expenses increase with debits; Liabilities, Equity, and Revenue increase with credits.</p>

<h3>Account Codes</h3>
<p>Each account has a numeric code that determines its category. MoneyMan uses the standard
small-business chart of accounts numbering:</p>
<ul>
  <li><code>1000</code> &mdash; Cash (Asset)</li>
  <li><code>2000</code> &mdash; Accounts Payable (Liability)</li>
  <li><code>3000</code> &mdash; Owner's Equity (Equity)</li>
  <li><code>4000</code> &mdash; Sales Revenue (Revenue)</li>
  <li><code>5000</code> &mdash; General Expenses (Expense)</li>
</ul>

<h3>Managing Accounts in MoneyMan</h3>
<p>The Chart of Accounts is displayed in the left dock panel. To add a new account,
click <b>Add Account</b> and fill in the code, name, and type. Accounts can be
toggled visible/hidden via the <b>Window</b> menu.</p>

<div class="warn"><b>Important:</b> Once an account has posted journal entries, it cannot be
deleted&mdash;only deactivated. This preserves the audit trail required by GAAP.</div>

<!-- ═══════════════════════════════════════════ -->
<a name="journal-entries"></a>
<h2>3. Journal Entries</h2>

<h3>The Fundamental Unit of Accounting</h3>
<p>A journal entry records a financial transaction. Every entry must have:</p>
<ul>
  <li>A <b>date</b> when the transaction occurred</li>
  <li>A <b>description</b> explaining what happened</li>
  <li>At least one <b>debit line</b> and one <b>credit line</b></li>
  <li>Total debits <b>must equal</b> total credits</li>
</ul>

<h3>Example: Recording a Sale</h3>
<table>
<tr><th>Account</th><th>Debit</th><th>Credit</th></tr>
<tr><td>1000 Cash</td><td>$500.00</td><td></td></tr>
<tr><td>4000 Sales Revenue</td><td></td><td>$500.00</td></tr>
</table>
<p>Cash (an asset) increases with a debit; Revenue increases with a credit.</p>

<h3>Posting Entries in MoneyMan</h3>
<p>Press <span class="key">Ctrl+J</span> or use <b>Transactions &gt; New Journal Entry</b>.
Add lines using the entry form, ensuring debits equal credits. Once posted, entries
cannot be deleted&mdash;only reversed with a correcting entry.</p>

<div class="tip"><b>Tip:</b> Use entry templates for recurring transactions like
monthly rent or payroll.</div>

<!-- ═══════════════════════════════════════════ -->
<a name="accounting-equation"></a>
<h2>4. The Accounting Equation</h2>

<p>The foundation of all double-entry bookkeeping is:</p>
<p style="text-align: center; font-size: 1.3em; margin: 20px 0;">
<b>Assets = Liabilities + Equity</b></p>

<p>Every transaction maintains this equation. When you record a journal entry,
the debits and credits ensure the equation stays in balance.</p>

<h3>Expanded Equation</h3>
<p style="text-align: center; font-size: 1.1em;">
Assets = Liabilities + Equity + (Revenue &minus; Expenses)</p>
<p>Revenue and Expenses are temporary accounts that feed into Equity at period close.
During the period, they expand the equation; at closing, their net is transferred
to Retained Earnings.</p>

<h3>How Debits and Credits Affect Accounts</h3>
<table>
<tr><th>Account Type</th><th>Debit Effect</th><th>Credit Effect</th></tr>
<tr><td>Assets</td><td>Increase (+)</td><td>Decrease (&minus;)</td></tr>
<tr><td>Liabilities</td><td>Decrease (&minus;)</td><td>Increase (+)</td></tr>
<tr><td>Equity</td><td>Decrease (&minus;)</td><td>Increase (+)</td></tr>
<tr><td>Revenue</td><td>Decrease (&minus;)</td><td>Increase (+)</td></tr>
<tr><td>Expenses</td><td>Increase (+)</td><td>Decrease (&minus;)</td></tr>
</table>

<!-- ═══════════════════════════════════════════ -->
<a name="trial-balance"></a>
<h2>5. Reports: Trial Balance</h2>

<h3>What the Trial Balance Proves</h3>
<p>The Trial Balance lists every account with its debit or credit balance. Its primary
purpose is to verify that <b>total debits equal total credits</b> across all accounts.</p>

<p>If the Trial Balance doesn't balance, there is an error in the journal entries.
Common causes include:</p>
<ul>
  <li>A journal entry that wasn't balanced (MoneyMan prevents this)</li>
  <li>A posting error to the wrong account</li>
  <li>A missing entry</li>
</ul>

<h3>Reading the Trial Balance in MoneyMan</h3>
<p>Navigate to the <b>Trial Balance</b> tab (<span class="key">Ctrl+2</span>).
Each account shows its balance in either the Debit or Credit column. The totals
at the bottom must match.</p>

<div class="tip"><b>Tip:</b> Export the Trial Balance to CSV or print it to PDF
using the export buttons for your records.</div>

<!-- ═══════════════════════════════════════════ -->
<a name="general-ledger"></a>
<h2>6. Reports: General Ledger</h2>

<h3>Complete Transaction History</h3>
<p>The General Ledger shows every journal entry that affected a particular account,
in chronological order. It provides the complete audit trail for each account.</p>

<h3>How to Audit with the General Ledger</h3>
<p>Navigate to the <b>General Ledger</b> tab (<span class="key">Ctrl+3</span>).
Select an account from the dropdown to see all its transactions. Each line shows
the date, description, debit/credit amount, and running balance.</p>

<p>Use the General Ledger to:</p>
<ul>
  <li>Trace the source of a balance discrepancy</li>
  <li>Review all transactions for a specific account</li>
  <li>Verify that entries were posted to the correct accounts</li>
</ul>

<!-- ═══════════════════════════════════════════ -->
<a name="income-statement"></a>
<h2>7. Reports: Income Statement</h2>

<h3>Revenue Minus Expenses Equals Net Income</h3>
<p>The Income Statement (also called Profit &amp; Loss) shows financial performance
over a period of time. It follows a simple structure:</p>

<p style="text-align: center; font-size: 1.1em;">
<b>Revenue &minus; Expenses = Net Income (or Net Loss)</b></p>

<h3>GAAP Matching Principle</h3>
<p>Under GAAP, expenses should be recognized in the same period as the revenue they
helped generate. This is the <b>matching principle</b>. For example, if you pay
for supplies in January that are used to deliver services in February, the expense
should be recorded in February.</p>

<h3>Using the Income Statement in MoneyMan</h3>
<p>Navigate to <b>Income Statement</b> (<span class="key">Ctrl+4</span>). The report
groups all Revenue accounts (4000-series) and Expense accounts (5000-series),
showing the net income or loss at the bottom.</p>

<!-- ═══════════════════════════════════════════ -->
<a name="balance-sheet"></a>
<h2>8. Reports: Balance Sheet</h2>

<h3>A Snapshot of Financial Position</h3>
<p>The Balance Sheet shows the financial position at a <b>specific point in time</b>.
It directly reflects the accounting equation:</p>

<p style="text-align: center; font-size: 1.1em;">
<b>Assets = Liabilities + Equity</b></p>

<p>Unlike the Income Statement (which covers a period), the Balance Sheet is a
snapshot&mdash;it shows what you own, what you owe, and the residual equity at
that moment.</p>

<h3>Using the Balance Sheet in MoneyMan</h3>
<p>Navigate to <b>Balance Sheet</b> (<span class="key">Ctrl+5</span>). The report
shows Assets (1000-series), Liabilities (2000-series), and Equity (3000-series)
with totals that must balance.</p>

<!-- ═══════════════════════════════════════════ -->
<a name="fiscal-periods"></a>
<h2>9. Fiscal Periods</h2>

<h3>Why Periods Are Closed</h3>
<p>At the end of each fiscal period (month, quarter, or year), temporary accounts
(Revenue and Expenses) are closed to Retained Earnings. This:</p>
<ul>
  <li>Resets Revenue and Expense accounts to zero for the new period</li>
  <li>Transfers net income/loss into Equity (Retained Earnings)</li>
  <li>Prevents modifications to prior-period entries</li>
</ul>

<h3>Closing Entries</h3>
<p>MoneyMan generates closing entries automatically when you close a period via
<b>File &gt; Close Period</b>. The process:</p>
<ol>
  <li>Debits all Revenue accounts to zero</li>
  <li>Credits all Expense accounts to zero</li>
  <li>Transfers the net difference to Retained Earnings (account 3200)</li>
</ol>

<div class="warn"><b>Warning:</b> Closing a period is irreversible. No new entries
can be posted to a closed period. Make sure all entries are recorded before closing.</div>

<!-- ═══════════════════════════════════════════ -->
<a name="data-management"></a>
<h2>10. Data Management</h2>

<h3>Encryption</h3>
<p>MoneyMan encrypts your database using SQLCipher (AES-256). You set a passphrase
on first launch, and it is required every time you open the application. Change your
passphrase via <b>File &gt; Change Encryption Key</b>.</p>

<h3>Backup &amp; Restore</h3>
<p>Regular backups protect against data loss:</p>
<ul>
  <li><b>File &gt; Backup Database</b> &mdash; saves an encrypted copy to a location you choose</li>
  <li><b>File &gt; Restore Database</b> &mdash; replaces current data with a backup (irreversible)</li>
</ul>

<h3>CSV Import</h3>
<p>Import a chart of accounts from a CSV file via <b>File &gt; Import Accounts CSV</b>.
The CSV must have columns for account code, name, and type.</p>

<h3>Exporting Reports</h3>
<p>All reports support export to CSV and PDF. Use the export buttons on each report
tab to save or print your financial data.</p>

<div class="tip"><b>Best practice:</b> Back up your database before closing a fiscal
period or importing data. Keep backups in a secure, separate location.</div>

</body>
</html>
)");
}
