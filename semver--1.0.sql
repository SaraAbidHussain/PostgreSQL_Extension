CREATE TYPE semver;

CREATE FUNCTION semver_in(cstring)
    RETURNS semver
    AS 'MODULE_PATHNAME', 'semver_in'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_out(semver)
    RETURNS cstring
    AS 'MODULE_PATHNAME', 'semver_out'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE semver (
    INPUT   = semver_in,
    OUTPUT  = semver_out,
    INTERNALLENGTH = 12,
    ALIGNMENT = int4
);