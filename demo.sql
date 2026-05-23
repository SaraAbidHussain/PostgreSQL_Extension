CREATE EXTENSION IF NOT EXISTS semver;

DROP TABLE IF EXISTS packages;

CREATE TABLE packages (
    name    text PRIMARY KEY,
    version semver NOT NULL
);

INSERT INTO packages VALUES
    ('postgres',      '16.4.0'),
    ('redis',         '7.2.5'),
    ('nginx',         '1.27.1'),
    ('node',          '20.10.0'),
    ('rust',          '1.81.0'),
    ('python',        '3.12.0'),
    ('go',            '1.22.0'),
    ('mysql',         '8.0.36'),
    ('mongodb',       '7.0.4'),
    ('kafka',         '3.6.0'),
    ('rabbitmq',      '3.12.0'),
    ('grafana',       '10.2.0'),
    ('prometheus',    '2.48.0'),
    ('docker',        '24.0.7'),
    ('kubernetes',    '1.28.4'),
    ('terraform',     '1.6.4'),
    ('vault',         '1.15.2'),
    ('consul',        '1.17.0'),
    ('elasticsearch', '8.11.0'),
    ('kibana',        '8.11.0');

SELECT '1.2.3'::semver;

SELECT '1.10.0'::semver > '1.9.0'::semver AS numeric_compare_correct;

SELECT name, version
FROM packages
WHERE version >= '2.0.0'::semver
ORDER BY version DESC;

SELECT major(version) AS major, minor(version) AS minor, patch(version) AS patch
FROM packages WHERE name = 'postgres';

SELECT max(version) FROM packages;
SELECT min(version) FROM packages;

SELECT name, version, is_compatible(version, '1.0.0'::semver)
FROM packages ORDER BY version DESC;

SELECT bump_minor('1.2.3'::semver);

CREATE INDEX idx_packages_version ON packages USING btree (version);

EXPLAIN SELECT * FROM packages WHERE version >= '2.0.0'::semver;

SET enable_seqscan = off;
EXPLAIN SELECT * FROM packages WHERE version >= '2.0.0'::semver;
SET enable_seqscan = on;

DROP TABLE packages;
