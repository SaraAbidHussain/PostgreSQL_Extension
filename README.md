# PostgreSQL `semver` Extension

A native PostgreSQL extension written in C that adds a `semver` (semantic version) data type to PostgreSQL. Store, validate, compare, and query version numbers like `1.2.3` directly in your database — correctly.

---

## What is Semantic Versioning?

```
MAJOR . MINOR . PATCH
  1   .   2   .   3
```

| Part | Meaning |
|---|---|
| **MAJOR** | Breaking changes |
| **MINOR** | New features, backwards compatible |
| **PATCH** | Bug fixes |

The key insight: `1.10.0 > 1.9.0` numerically, but a naive string comparison gets this wrong. This extension fixes that at the database level.

---

## System Requirements

- Ubuntu 22.04 / 24.04
- PostgreSQL 16
- GCC (C compiler)
- Make

```bash
sudo apt update
sudo apt install postgresql postgresql-server-dev-all build-essential
```

Verify setup:
```bash
pg_config --includedir-server   # must print a valid path
pg_config --pkglibdir           # must print a valid path
```

---

## Project Structure

```
PostgreSQL_Extension/
├── semver.c              # All C code: type, comparisons, utilities, aggregates
├── semver.control        # Extension metadata
├── semver--1.0.sql       # SQL registrations (runs on CREATE EXTENSION)
├── Makefile              # PGXS build configuration
├── sql/
│   └── semver_test.sql   # Regression test queries
├── expected/
│   └── semver_test.out   # Expected regression test output
├── demo.sql              # Full demo with realistic dataset
└── README.md             # This file
```

---

## Build and Install

```bash
# 1. Compile
make

# 2. Install into PostgreSQL
sudo make install

# 3. Create a test database (first time only)
createdb mydb

# 4. Load the extension
psql mydb -c "CREATE EXTENSION semver;"
```

> **Reinstalling after changes:** Always drop and recreate the extension after rebuilding.
> ```bash
> psql mydb -c "DROP EXTENSION IF EXISTS semver CASCADE;"
> make && sudo make install
> psql mydb -c "CREATE EXTENSION semver;"
> ```

---

## Supported Features

### The `semver` Data Type

Stored internally as a struct of three 32-bit integers — 12 bytes per value:

```c
typedef struct {
    int32 major;
    int32 minor;
    int32 patch;
} Semver;
```

### Comparison Operators

| Operator | Example | Result |
|---|---|---|
| `<` | `'1.2.3'::semver < '1.10.0'::semver` | `t` |
| `<=` | `'1.2.3'::semver <= '1.2.3'::semver` | `t` |
| `=` | `'2.0.0'::semver = '2.0.0'::semver` | `t` |
| `<>` | `'1.0.0'::semver <> '1.0.1'::semver` | `t` |
| `>=` | `'1.10.0'::semver >= '1.9.0'::semver` | `t` |
| `>` | `'1.10.0'::semver > '1.9.0'::semver` | `t` |

All comparisons are **integer-based field by field** (major → minor → patch), never lexicographic.

### Utility Functions

| Function | Returns | Description |
|---|---|---|
| `major(semver)` | `int` | Extracts the major component |
| `minor(semver)` | `int` | Extracts the minor component |
| `patch(semver)` | `int` | Extracts the patch component |
| `bump_minor(semver)` | `semver` | Increments minor, resets patch to 0 |
| `is_compatible(a, b)` | `bool` | True if same major and `b <= a` (npm `^` rule) |

### Aggregates

| Aggregate | Description |
|---|---|
| `max(semver)` | Returns the highest version in a set |
| `min(semver)` | Returns the lowest version in a set |

### Input Validation

The following inputs are rejected with a clear error:

| Invalid Input | Reason |
|---|---|
| `'1.2'` | Missing patch component |
| `'1.2.3.4'` | Too many components |
| `'v1.2.3'` | Prefix not allowed |
| `'1.2.x'` | Non-integer component |
| `'-1.2.3'` | Negative numbers not allowed |
| `'1.0.0-rc.1'` | Pre-release labels not supported |

---

## Usage Examples

```sql
CREATE EXTENSION semver;

-- Basic input and output
SELECT '1.2.3'::semver;       -- 1.2.3
SELECT '20.10.0'::semver;     -- 20.10.0

-- Comparisons (integer-based, not string-based)
SELECT '1.10.0'::semver > '1.9.0'::semver;    -- t  (correct!)
SELECT '1.2.3'::semver < '1.10.0'::semver;    -- t

-- Create a table
CREATE TABLE packages (
    name    text PRIMARY KEY,
    version semver NOT NULL
);

INSERT INTO packages VALUES
    ('postgres', '16.4.0'),
    ('redis',    '7.2.5'),
    ('nginx',    '1.27.1'),
    ('node',     '20.10.0'),
    ('rust',     '1.81.0');

-- Sort correctly
SELECT * FROM packages ORDER BY version DESC;
--   node     | 20.10.0
--   postgres | 16.4.0
--   redis    | 7.2.5
--   rust     | 1.81.0
--   nginx    | 1.27.1

-- Filter with operators
SELECT * FROM packages WHERE version >= '2.0.0'::semver;
--   postgres | 16.4.0
--   redis    | 7.2.5
--   node     | 20.10.0

-- Extract components
SELECT major(version), minor(version), patch(version)
FROM packages WHERE name = 'postgres';
--  16 | 4 | 0

-- Bump a version
SELECT bump_minor('1.2.3'::semver);   -- 1.3.0
SELECT bump_minor('1.9.3'::semver);   -- 1.10.0

-- Compatibility check (npm caret ^ rule)
SELECT is_compatible('1.5.0'::semver, '1.4.7'::semver);   -- t
SELECT is_compatible('2.0.0'::semver, '1.9.0'::semver);   -- f

-- Aggregates
SELECT max(version) FROM packages;    -- 20.10.0
SELECT min(version) FROM packages;    -- 1.27.1
```

---

## Running Tests

```bash
make installcheck
```

This runs `sql/semver_test.sql` against the installed extension and compares output character-for-character with `expected/semver_test.out`. All tests must pass before submission.

---

## Key Concepts

### `PG_MODULE_MAGIC`
A required stamp in exactly one `.c` file. PostgreSQL checks this to confirm the library was compiled against the same version that is running. Missing it causes the server to refuse loading the extension.

### `palloc` vs `malloc`
PostgreSQL uses its own memory allocator. `palloc` memory is automatically freed when a query ends. Using `malloc` causes memory leaks; mixing `malloc` with `pfree` crashes the server.

### `Datum`
PostgreSQL passes all values between functions as `Datum` — a universal pointer-sized container. The `PG_GETARG_*` and `PG_RETURN_*` macros handle conversion to and from real C types.

### `PGXS`
PostgreSQL's built-in build system. The `Makefile` uses PGXS to find the right compiler flags, headers, and install paths automatically.

### Operator Class
Registered with `CREATE OPERATOR CLASS ... USING btree` so PostgreSQL knows how to sort `semver` values. Required for `ORDER BY`, `MIN`, `MAX`, and any future index support.

---

## Known Limitations

- Pre-release labels not supported (`1.0.0-rc.1`)
- Build metadata not supported (`1.0.0+build123`)
- No index support yet (B-tree operator class registered for sorting, but `CREATE INDEX` not tested)

---

## Project Status

| Phase | Status | Description |
|---|---|---|
| Phase 1 | Complete | Type definition, input/output, table storage, validation |
| Phase 2 | Complete | Comparison operators, utility functions, ORDER BY support |
| Phase 3 | 🔜 In Progress | MAX/MIN aggregates, regression tests, demo script |

---

## Authors

**Sara Abid Hussain** · **Ammara Khan**

Course: Advanced Database Management Systems
Semester: 4th — Computer Science
Project: 02 — PostgreSQL Extension in C