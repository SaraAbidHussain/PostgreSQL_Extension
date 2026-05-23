#include "postgres.h"  
#include "fmgr.h"         
#include "utils/builtins.h" 

PG_MODULE_MAGIC;

typedef struct {
    int32 major;
    int32 minor;
    int32 patch;
} Semver;

PG_FUNCTION_INFO_V1(semver_in);
Datum semver_in(PG_FUNCTION_ARGS)
{
    char   *str    = PG_GETARG_CSTRING(0);
    
    Semver *result = (Semver *) palloc(sizeof(Semver));
    
    int     n;
    char    extra; 

    n = sscanf(str, "%d.%d.%d%c",
               &result->major,
               &result->minor,
               &result->patch,
               &extra);

    if (n != 3 || result->major < 0 || result->minor < 0 || result->patch < 0)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for type semver: \"%s\"", str),
                 errhint("Expected format: MAJOR.MINOR.PATCH (e.g. 1.2.3)")));

    PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(semver_out);
Datum semver_out(PG_FUNCTION_ARGS)
{
    Semver *v      = (Semver *) PG_GETARG_POINTER(0);
    
    char   *result = (char *) palloc(32);

    snprintf(result, 32, "%d.%d.%d", v->major, v->minor, v->patch);

    PG_RETURN_CSTRING(result);
}

static int
semver_compare(Semver *a, Semver *b)
{
    if (a->major != b->major)
        return a->major - b->major;
    if (a->minor != b->minor)
        return a->minor - b->minor;
    return a->patch - b->patch;
}


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
    bool compatible = (a->major == b->major) && (semver_compare(a, b) >= 0);
    PG_RETURN_BOOL(compatible);
}


PG_FUNCTION_INFO_V1(semver_larger);
Datum semver_larger(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_POINTER(semver_compare(a, b) >= 0 ? a : b);
}

PG_FUNCTION_INFO_V1(semver_smaller);
Datum semver_smaller(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_POINTER(semver_compare(a, b) <= 0 ? a : b);
}


//BONUS 3: Cast functions between text and semver

PG_FUNCTION_INFO_V1(semver_from_text);
Datum semver_from_text(PG_FUNCTION_ARGS)
{
    text   *txt = PG_GETARG_TEXT_PP(0);
    char   *str = text_to_cstring(txt);
    Semver *result = (Semver *) palloc(sizeof(Semver));
    int     n;
    char    extra;

    n = sscanf(str, "%d.%d.%d%c",
               &result->major,
               &result->minor,
               &result->patch,
               &extra);

    if (n != 3 || result->major < 0 || result->minor < 0 || result->patch < 0)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for type semver: \"%s\"", str),
                 errhint("Expected format: MAJOR.MINOR.PATCH (e.g. 1.2.3)")));

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(semver_to_text);
Datum semver_to_text(PG_FUNCTION_ARGS)
{
    Semver *v      = (Semver *) PG_GETARG_POINTER(0);
    char   *str    = (char *) palloc(32);
    snprintf(str, 32, "%d.%d.%d", v->major, v->minor, v->patch);
    PG_RETURN_TEXT_P(cstring_to_text(str));
}