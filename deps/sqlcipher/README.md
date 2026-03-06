# Bundled SQLCipher for Windows

Pre-built SQLCipher binaries for the Windows CI/CD pipeline.

## Directory Layout

```
win64/
  include/sqlcipher/sqlite3.h   -- SQLCipher/SQLite3 header
  lib/sqlcipher.lib             -- MSVC import library
  bin/sqlcipher.dll             -- Runtime DLL (copied next to moneyman.exe)
```

## Updating

1. Build SQLCipher for Windows (MSVC x64) or download a pre-built release
2. Replace the files in `win64/` with the new versions
3. Commit and push

## Linux / macOS

These platforms use system-installed SQLCipher via PkgConfig:
- Linux: `sudo apt-get install libsqlcipher-dev`
- macOS: `brew install sqlcipher`
