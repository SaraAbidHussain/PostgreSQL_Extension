#include "postgres.h"       /* Core PostgreSQL types and macros */
#include "fmgr.h"           /* Function manager - needed for PG_FUNCTION_INFO_V1 */
#include "utils/builtins.h" /* Utility functions */

/* 
 * MAGIC BLOCK - Must appear in exactly ONE .c file
 * PostgreSQL checks this to confirm the library was compiled
 * against the same PostgreSQL version that is running
 */
PG_MODULE_MAGIC;

/*
 * Our data structure - 3 integers = 12 bytes total
 * This is what gets stored on disk for each semver value
 */
typedef struct {
    int32 major;
    int32 minor;
    int32 patch;
} Semver;


/* ================================================
 * semver_in: called when user writes '1.2.3'::semver
 * Input:  C string like "1.2.3"
 * Output: pointer to a Semver struct
 * ================================================ */
PG_FUNCTION_INFO_V1(semver_in);
Datum semver_in(PG_FUNCTION_ARGS)
{
    /* Get the raw string PostgreSQL passed us */
    char   *str    = PG_GETARG_CSTRING(0);
    
    /* Allocate memory using palloc (NOT malloc!) */
    Semver *result = (Semver *) palloc(sizeof(Semver));
    
    int     n;
    char    extra; /* used to detect extra characters after patch */

    /*
     * sscanf tries to parse "major.minor.patch"
     * The %c at the end catches any extra characters like "1.2.3abc"
     * We want exactly 3 integers and nothing else
     */
    n = sscanf(str, "%d.%d.%d%c",
               &result->major,
               &result->minor,
               &result->patch,
               &extra);

    /*
     * n must be exactly 3 (not 4, which would mean extra chars were found)
     * Also reject negative numbers
     */
    if (n != 3 || result->major < 0 || result->minor < 0 || result->patch < 0)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for type semver: \"%s\"", str),
                 errhint("Expected format: MAJOR.MINOR.PATCH (e.g. 1.2.3)")));

    PG_RETURN_POINTER(result);
}


/* ================================================
 * semver_out: called when PostgreSQL needs to display a semver
 * Input:  pointer to a Semver struct
 * Output: C string like "1.2.3"
 * ================================================ */
PG_FUNCTION_INFO_V1(semver_out);
Datum semver_out(PG_FUNCTION_ARGS)
{
    /* Get our struct back from PostgreSQL */
    Semver *v      = (Semver *) PG_GETARG_POINTER(0);
    
    /* Allocate output buffer with palloc (NOT malloc!) */
    /* 32 bytes is more than enough for "XXXXXXXXXX.XXXXXXXXXX.XXXXXXXXXX" */
    char   *result = (char *) palloc(32);

    /* Format it back as "major.minor.patch" */
    snprintf(result, 32, "%d.%d.%d", v->major, v->minor, v->patch);

    PG_RETURN_CSTRING(result);
}

/* ================================================
 * INTERNAL HELPER - not called from SQL directly
 * Returns negative if a < b, 0 if equal, positive if a > b
 * This is the core comparison logic used by all operators
 * ================================================ */
static int
semver_compare(Semver *a, Semver *b)
{
    if (a->major != b->major)
        return a->major - b->major;
    if (a->minor != b->minor)
        return a->minor - b->minor;
    return a->patch - b->patch;
}

/* ================================================
 * COMPARISON FUNCTIONS - all follow the same pattern
 * Get two semver pointers, compare them, return bool or int
 * ================================================ */

PG_FUNCTION_INFO_V1(semver_lt);
Datum semver_lt(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) < 0);
}

PG_FUNCTION_INFO_V1(semver_le);
Datum semver_le(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(semver_eq);
Datum semver_eq(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) == 0);
}

PG_FUNCTION_INFO_V1(semver_ne);
Datum semver_ne(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) != 0);
}

PG_FUNCTION_INFO_V1(semver_ge);
Datum semver_ge(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(semver_gt);
Datum semver_gt(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) > 0);
}

PG_FUNCTION_INFO_V1(semver_cmp);
Datum semver_cmp(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_INT32(semver_compare(a, b));
}

/* ================================================
 * UTILITY FUNCTIONS
 * ================================================ */

PG_FUNCTION_INFO_V1(semver_major);
Datum semver_major(PG_FUNCTION_ARGS)
{
    Semver *v = (Semver *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(v->major);
}

PG_FUNCTION_INFO_V1(semver_minor);
Datum semver_minor(PG_FUNCTION_ARGS)
{
    Semver *v = (Semver *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(v->minor);
}

PG_FUNCTION_INFO_V1(semver_patch);
Datum semver_patch(PG_FUNCTION_ARGS)
{
    Semver *v = (Semver *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(v->patch);
}

PG_FUNCTION_INFO_V1(semver_bump_minor);
Datum semver_bump_minor(PG_FUNCTION_ARGS)
{
    Semver *v      = (Semver *) PG_GETARG_POINTER(0);
    Semver *result = (Semver *) palloc(sizeof(Semver));
    result->major  = v->major;
    result->minor  = v->minor + 1;
    result->patch  = 0;
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(semver_is_compatible);
Datum semver_is_compatible(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    /*
     * Compatible means: same major number, and b <= a in minor.patch
     * i.e. a has everything b had plus more (npm caret rule)
     */
    bool compatible = (a->major == b->major) && (semver_compare(a, b) >= 0);
    PG_RETURN_BOOL(compatible);
}


