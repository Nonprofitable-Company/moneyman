# Source Document Attachments — Design

**Date:** 2026-03-06
**Issue:** #5

## Problem

Users need to attach source documents (receipts, invoices, bank statements) to journal entries. Currently there's no way to link them, requiring a separate filing system.

## Decisions

- **Storage:** Embedded BLOBs in SQLite (encrypted at rest by SQLCipher, single-file backups)
- **File types:** PNG, JPG, PDF only
- **Access points:** Journal Entry dialog (new entries) + retroactively from Journal List (existing entries)

## Database Schema

```sql
CREATE TABLE attachments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    entry_id INTEGER NOT NULL REFERENCES journal_entries(id),
    filename TEXT NOT NULL,
    mime_type TEXT NOT NULL,
    data BLOB NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);
```

## DB Layer API

```cpp
struct AttachmentRow {
    int64_t id = 0;
    int64_t entryId = 0;
    QString filename;
    QString mimeType;
    QByteArray data;
    QString createdAt;
};

struct AttachmentMeta {
    int64_t id = 0;
    QString filename;
    QString mimeType;
    QString createdAt;
};

bool addAttachment(int64_t entryId, const QString &filename,
                   const QString &mimeType, const QByteArray &data);
std::vector<AttachmentMeta> attachmentsForEntry(int64_t entryId) const;
AttachmentRow attachmentById(int64_t id) const;
bool deleteAttachment(int64_t id);
```

Split between `AttachmentMeta` (listing) and `AttachmentRow` (fetching) avoids loading BLOBs just to show filenames.

## UI

### Journal Entry Dialog (new entries)
- "Attach File..." button in the button row below the lines table
- List of attached filenames with remove button
- Files held in memory until entry is posted

### Journal List (existing entries)
- Paperclip icon column for entries with attachments
- Right-click context menu: "Attachments..." action

### Attachments Dialog (shared)
- List of attached files: filename, date added
- "Open" — save to temp file, open with `QDesktopServices::openUrl`
- "Add..." — file picker filtered to `*.png *.jpg *.jpeg *.pdf`
- "Remove" — delete attachment

## Testing

New tests in `test_db`:
- Add attachment, verify retrieval
- List attachments returns metadata without BLOB
- Delete attachment removes it
- Foreign key references valid journal entry
