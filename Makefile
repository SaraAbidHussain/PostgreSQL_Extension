MODULE_big = semver
OBJS = semver.o
EXTENSION = semver
DATA = semver--1.0.sql
REGRESS = semver_test

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)