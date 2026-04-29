

```
# PostgreSQL semver Extension

A native PostgreSQL extension written in C that adds a `semver` (semantic version) data type to PostgreSQL. This extension allows you to store, validate, and manipulate version numbers like `1.2.3` directly in your database.

---

## What is Semantic Versioning?

Semantic versioning is a versioning format used by software packages:

```
MAJOR . MINOR . PATCH
  1   .   2   .   3
```

- **MAJOR** — breaking changes
- **MINOR** — new features added
- **PATCH** — bug fixes

Examples: `1.0.0`, `16.4.0`, `20.10.0`

---

## System Requirements

- Ubuntu 22.04 / 24.04
- PostgreSQL 16
- GCC (C compiler)
- Make

Install all dependencies with:

```bash
sudo apt update
sudo apt install postgresql postgresql-server-dev-all build-essential
```

---

## Project Structure

```
PostgreSQL_Extension/
├── semver.c            # Core C code: type definition, input/output functions
├── semver.control      # Extension metadata (name, version, description)
├── semver--1.0.sql     # SQL registration script (runs on CREATE EXTENSION)
├── Makefile            # PGXS build system configuration
├── sql/
│   └── semver_test.sql # Regression test queries
├── expected/
│   └── semver_test.out # Expected regression test output
├── demo.sql            # Full demo script with realistic dataset
└── README.md           # This file
```

---

## Build and Install

```bash
# Step 1: Compile the extension
make

# Step 2: Install into PostgreSQL
sudo make install

# Step 3: Create a test database (first time only)
createdb mydb

# Step 4: Load the extension
psql mydb -c "CREATE EXTENSION semver;"
```

---

## Phase 1 — What is Implemented

### The `semver` Data Type

A new PostgreSQL type that stores semantic version numbers as a struct of three 32-bit integers:

```c
typedef struct {
    int32 major;
    int32 minor;
    int32 patch;
} Semver;
```

Total storage: **12 bytes** per value.

### Functions Implemented

| Function | Description |
|---|---|
| `semver_in(cstring)` | Parses text like `"1.2.3"` into a Semver struct |
| `semver_out(semver)` | Converts a Semver struct back to text `"1.2.3"` |

### Input Validation

The following inputs are correctly rejected with a clear error message:

| Invalid Input | Reason |
|---|---|
| `'1.2'` | Missing patch component |
| `'1.2.3.4'` | Too many components |
| `'v1.2.3'` | Prefix not allowed |
| `'1.2.x'` | Non-integer component |
| `''` | Empty string |
| `'-1.2.3'` | Negative numbers not allowed |

---

## Phase 1 Usage Examples

```sql
-- Load the extension
CREATE EXTENSION semver;

-- Basic input and output
SELECT '1.2.3'::semver;      -- returns: 1.2.3
SELECT '16.4.0'::semver;     -- returns: 16.4.0
SELECT '20.10.0'::semver;    -- returns: 20.10.0

-- Invalid input (gives clear error)
SELECT '1.2.x'::semver;
-- ERROR: invalid input syntax for type semver: "1.2.x"
-- HINT:  Expected format: MAJOR.MINOR.PATCH (e.g. 1.2.3)

-- Store in a table
CREATE TABLE packages (
    name    text PRIMARY KEY,
    version semver NOT NULL
);

INSERT INTO packages VALUES
    ('postgres', '16.4.0'),
    ('redis',    '7.2.5'),
    ('nginx',    '1.27.1');

SELECT * FROM packages;
```

---

## Key Concepts Used

### PG_MODULE_MAGIC
A required stamp in every PostgreSQL extension. It tells PostgreSQL that this library was compiled against the same version of PostgreSQL that is running. Without it, PostgreSQL refuses to load the extension.

### palloc vs malloc
PostgreSQL uses its own memory allocator called `palloc`. Unlike `malloc`, memory allocated with `palloc` is automatically freed when the query finishes. Using `malloc` inside PostgreSQL functions causes memory leaks.

### Datum
PostgreSQL passes all values between functions as `Datum` — a universal container type. The macros `PG_GETARG_*` and `PG_RETURN_*` handle packing and unpacking values into Datum automatically.

### PGXS
PostgreSQL's built-in build system. The `Makefile` uses PGXS to automatically find the right compiler flags, header files, and installation directories. You just run `make` and it handles everything.

---

## Phases Overview

| Phase | Status | Description |
|---|---|---|
| Phase 1 | Complete | Type definition, input/output, table storage |
| Phase 2 | 🔜 In Progress | Comparison operators, utility functions |
| Phase 3 | 🔜 Pending | Aggregates, regression tests, demo script |

---

## Known Limitations (Phase 1)

- No comparison operators yet (`<`, `>`, `=`) — coming in Phase 2
- No sorting support yet — coming in Phase 2
- Pre-release labels not supported (e.g. `1.0.0-rc.1`)
- Build metadata not supported (e.g. `1.0.0+build123`)

---

## Authors

- Sara Abid Hussain
- Ammara Khan

**Course:** Advanced Database Management Systems  
**Semester:** 4th — Computer Science  
**Project:** 02 — PostgreSQL Extension in C
```

