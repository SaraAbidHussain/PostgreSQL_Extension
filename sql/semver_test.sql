CREATE EXTENSION semver;

-- ================================================
-- semver regression tests
-- Covers all requirements from the project manual
-- ================================================

-- ================================================
-- 1. BASIC INPUT AND OUTPUT
-- ================================================

SELECT '1.2.3'::semver;
SELECT '0.0.1'::semver;
SELECT '16.4.0'::semver;
SELECT '20.10.0'::semver;
SELECT '0.0.0'::semver;

-- ================================================
-- 2. INPUT REJECTION (malformed strings)
-- ================================================

SELECT '1.2'::semver;
SELECT '1.2.3.4'::semver;
SELECT 'v1.2.3'::semver;
SELECT '1.2.x'::semver;
SELECT ''::semver;
SELECT '-1.2.3'::semver;
SELECT '1.2.3-rc.1'::semver;
SELECT '1.2.3+build'::semver;

-- ================================================
-- 3. COMPARISON OPERATORS
-- field-by-field integer comparison, not string
-- ================================================

SELECT '1.2.3'::semver < '1.2.4'::semver AS lt_patch;
SELECT '1.2.3'::semver < '1.3.0'::semver AS lt_minor;
SELECT '1.2.3'::semver < '2.0.0'::semver AS lt_major;
SELECT '1.9.0'::semver < '1.10.0'::semver AS lt_critical;

SELECT '1.2.3'::semver <= '1.2.3'::semver AS le_equal;
SELECT '1.2.3'::semver <= '1.2.4'::semver AS le_less;

SELECT '1.2.3'::semver = '1.2.3'::semver AS eq_true;
SELECT '1.2.3'::semver = '1.2.4'::semver AS eq_false;

SELECT '1.2.3'::semver <> '1.2.4'::semver AS ne_true;
SELECT '1.2.3'::semver <> '1.2.3'::semver AS ne_false;

SELECT '1.2.3'::semver >= '1.2.3'::semver AS ge_equal;
SELECT '1.10.0'::semver >= '1.9.0'::semver AS ge_critical;

SELECT '1.10.0'::semver > '1.9.0'::semver AS gt_critical;
SELECT '2.0.0'::semver > '1.99.99'::semver AS gt_major;

-- ================================================
-- 4. UTILITY FUNCTIONS
-- ================================================

SELECT major('16.4.0'::semver);
SELECT minor('16.4.0'::semver);
SELECT patch('16.4.0'::semver);

SELECT bump_minor('1.2.3'::semver);
SELECT bump_minor('1.9.3'::semver);
SELECT bump_minor('0.0.1'::semver);

SELECT is_compatible('1.5.0'::semver, '1.4.7'::semver);
SELECT is_compatible('1.4.7'::semver, '1.5.0'::semver);
SELECT is_compatible('2.0.0'::semver, '1.9.0'::semver);
SELECT is_compatible('1.0.0'::semver, '1.0.0'::semver);

-- ================================================
-- 5. TABLE OPERATIONS AND ORDER BY
-- ================================================

CREATE TABLE test_packages (name text, version semver);

INSERT INTO test_packages VALUES
    ('postgres', '16.4.0'),
    ('redis',    '7.2.5'),
    ('nginx',    '1.27.1'),
    ('node',     '20.10.0'),
    ('rust',     '1.81.0'),
    ('python',   '3.12.0'),
    ('go',       '1.22.0');

SELECT * FROM test_packages ORDER BY version ASC;
SELECT * FROM test_packages ORDER BY version DESC;

SELECT * FROM test_packages WHERE version >= '2.0.0'::semver ORDER BY version DESC;
SELECT * FROM test_packages WHERE version < '2.0.0'::semver ORDER BY version ASC;

-- ================================================
-- 6. MAX AND MIN AGGREGATES
-- ================================================

SELECT max(version) FROM test_packages;
SELECT min(version) FROM test_packages;

-- ================================================
-- 7. COMBINED QUERY (is_compatible demo)
-- ================================================

SELECT name, version, is_compatible(version, '1.0.0'::semver)
FROM test_packages
ORDER BY version DESC;

-- ================================================
-- cleanup
-- ================================================

DROP TABLE test_packages;
