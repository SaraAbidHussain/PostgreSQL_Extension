Here is the complete updated README.md. Open `README.md` in VS Code and replace everything with this:

```markdown
# PostgreSQL semver Extension

A native PostgreSQL extension written in C that adds a `semver` (semantic version) data type to PostgreSQL. Store, validate, compare, and query version numbers like `1.2.3` directly in your database with correct integer-based comparison.

---

## What is Semantic Versioning?

```
MAJOR . MINOR . PATCH
  1   .   2   .   3
```

| Part | Meaning |
|---|---|
| MAJOR | Breaking changes |
| MINOR | New features, backwards compatible |
| PATCH | Bug fixes |

The key problem this extension solves: `1.10.0 > 1.9.0` numerically, but a naive string comparison gets this wrong. This extension fixes that at the database level by comparing each part as an integer.

---

## System Requirements

- Ubuntu 22.04 or 24.04
- PostgreSQL 16
- GCC compiler
- Make

Install all dependencies:

```bash
sudo apt update
sudo apt install postgresql postgresql-server-dev-all build-essential
```

Verify setup:

```bash
pg_config --includedir-server
pg_config --pkglibdir
```

Both commands must print valid directory paths.

---

## Project Structure

```
PostgreSQL_Extension/
├── semver.c                 All C code: type, comparisons, utilities, aggregates, casts
├── semver.control           Extension metadata
├── semver--1.0.sql          SQL registrations, runs on CREATE EXTENSION
├── Makefile                 PGXS build configuration
├── sql/
│   └── semver_test.sql      Regression test queries
├── expected/
│   └── semver_test.out      Expected regression test output
├── demo.sql                 Full demo with 20 real software packages
└── README.md                This file
```

---

## Build and Install

```bash
# Step 1: Compile
make

# Step 2: Install into PostgreSQL
sudo make install

# Step 3: Create a test database (first time only)
createdb mydb

# Step 4: Load the extension
psql mydb -c "CREATE EXTENSION semver;"
```

After making any code changes, reinstall like this:

```bash
psql mydb -c "DROP EXTENSION IF EXISTS semver CASCADE;"
make && sudo make install
psql mydb -c "CREATE EXTENSION semver;"
```

---

## Supported Types

| Type | Storage | Description |
|---|---|---|
| `semver` | 12 bytes | Semantic version number stored as three int32 fields |

---

## Supported Functions

| Function | Returns | Description |
|---|---|---|
| `major(semver)` | `int` | Returns the major component |
| `minor(semver)` | `int` | Returns the minor component |
| `patch(semver)` | `int` | Returns the patch component |
| `bump_minor(semver)` | `semver` | Increments minor by 1 and resets patch to 0 |
| `is_compatible(a, b)` | `bool` | True if same major and b is less than or equal to a |
| `semver_cmp(a, b)` | `int` | Returns negative, zero, or positive for sorting |
| `semver_from_text(text)` | `semver` | Cast function from text to semver |
| `semver_to_text(semver)` | `text` | Cast function from semver to text |

---

## Supported Operators

| Operator | Description | Example |
|---|---|---|
| `<` | Less than | `'1.2.3'::semver < '1.10.0'::semver` |
| `<=` | Less than or equal | `'1.2.3'::semver <= '1.2.3'::semver` |
| `=` | Equal | `'2.0.0'::semver = '2.0.0'::semver` |
| `<>` | Not equal | `'1.0.0'::semver <> '1.0.1'::semver` |
| `>=` | Greater than or equal | `'1.10.0'::semver >= '1.9.0'::semver` |
| `>` | Greater than | `'1.10.0'::semver > '1.9.0'::semver` |

All comparisons are integer-based field by field, never string-based.

---

## Supported Aggregates

| Aggregate | Description |
|---|---|
| `max(semver)` | Returns the highest version in a column |
| `min(semver)` | Returns the lowest version in a column |

---

## Supported Casts (Bonus)

| Cast | Description |
|---|---|
| `text::semver` | Convert a text value to semver |
| `semver::text` | Convert a semver value to text |

---

## Input Validation

The following inputs are rejected with a clear error message:

| Invalid Input | Reason |
|---|---|
| `'1.2'` | Missing patch component |
| `'1.2.3.4'` | Too many components |
| `'v1.2.3'` | Prefix not allowed |
| `'1.2.x'` | Non-integer component |
| `''` | Empty string |
| `'-1.2.3'` | Negative numbers not allowed |
| `'1.0.0-rc.1'` | Pre-release labels not supported |
| `'1.0.0+build'` | Build metadata not supported |

---

## Usage Examples

```sql
CREATE EXTENSION semver;

SELECT '1.2.3'::semver;        
SELECT '20.10.0'::semver;     

SELECT '1.10.0'::semver > '1.9.0'::semver;  
SELECT '1.2.3'::semver < '1.10.0'::semver;   

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

SELECT * FROM packages ORDER BY version DESC;

SELECT * FROM packages WHERE version >= '2.0.0'::semver;

SELECT major(version), minor(version), patch(version)
FROM packages WHERE name = 'postgres';

SELECT bump_minor('1.2.3'::semver);  
SELECT bump_minor('1.9.3'::semver);  

SELECT is_compatible('1.5.0'::semver, '1.4.7'::semver);   
SELECT is_compatible('2.0.0'::semver, '1.9.0'::semver);   

SELECT max(version) FROM packages;  
SELECT min(version) FROM packages;  

SELECT '1.2.3'::text::semver;   
SELECT '1.2.3'::semver::text;   

CREATE INDEX idx_version ON packages USING btree (version);
EXPLAIN SELECT * FROM packages WHERE version >= '2.0.0'::semver;
```

---

## Running the Demo

```bash
psql mydb -f demo.sql
```

This loads 20 real software packages and demonstrates all features of the extension in one screen of output.

---

## Running Regression Tests

```bash
make installcheck
```

This runs sql/semver_test.sql against the installed extension and compares output character by character with expected/semver_test.out. All tests must pass.

Expected result:

```
All 1 tests passed.
```

---

## Key Concepts

**PG_MODULE_MAGIC** is a required stamp in exactly one .c file. PostgreSQL checks this to confirm the library was compiled against the same version that is running. Missing it causes the server to refuse loading.

**palloc vs malloc** is an important rule. PostgreSQL uses its own memory allocator. palloc memory is automatically freed when a query ends. Using malloc causes memory leaks. Mixing malloc with pfree crashes the server. We use palloc everywhere.

**Datum** is PostgreSQL's universal container type. All values passed between functions go through Datum. The PG_GETARG and PG_RETURN macros handle the conversion automatically.

**PGXS** is PostgreSQL's built-in build system. Our Makefile uses PGXS to find the right compiler flags, headers, and install paths automatically with just four lines of configuration.

---

## Known Limitations

- Pre-release labels not supported such as 1.0.0-rc.1
- Build metadata not supported such as 1.0.0+build123
- Maximum value per component is 2,147,483,647

