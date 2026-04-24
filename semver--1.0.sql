-- semver--1.0.sql
-- Runs automatically when user types: CREATE EXTENSION semver

-- 1. Declare a "shell" type first (empty placeholder)
--    We need this because semver_in and semver_out reference each other
CREATE TYPE semver;

-- 2. Tell PostgreSQL: "semver_in" is a C function in our library
--    It takes raw text input and returns a semver value
CREATE FUNCTION semver_in(cstring)
    RETURNS semver
    AS 'MODULE_PATHNAME', 'semver_in'
    LANGUAGE C IMMUTABLE STRICT;

-- 3. Tell PostgreSQL: "semver_out" converts semver back to text
CREATE FUNCTION semver_out(semver)
    RETURNS cstring
    AS 'MODULE_PATHNAME', 'semver_out'
    LANGUAGE C IMMUTABLE STRICT;

-- 4. Now complete the type with its input/output functions
--    INTERNALLENGTH = 12 because our struct has 3 x int32 (4 bytes each)
--    ALIGNMENT = int4 means align to 4-byte boundaries
CREATE TYPE semver (
    INPUT   = semver_in,
    OUTPUT  = semver_out,
    INTERNALLENGTH = 12,
    ALIGNMENT = int4
);