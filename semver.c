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