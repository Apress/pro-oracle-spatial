/* read_geom.c

   This program reads geometry objects from a table in the database.

   The geometry objects can be of any kind. Each object is first
   loaded into a C structure, from which it is then printed out.

   The input to the program is a SELECT statement that returns a single
   column of type SDO_GEOMETRY.

   It illustrates the following concepts:
   - passing SQL statements from the command line
   - reading and decoding geometry objects

   The program takes the following command line arguments:

     read_geom username password database select_statement print_level

   where

   - username = name of the user to connect as
   - password = password for that user
   - database = TNS service name for the database
   - print_level = level of detail to print out
     0 = do not print geometries
     1 = print summary (type, number of elements, number of points)
     2 = print details (all elements and point details)

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>
#include "sdo_geometry.h"

#define use_array_interface 0

/*******************************************************************************
** Global variables
*******************************************************************************/

/* OCI handles */

OCIEnv       *envhp;  /* Environment handle*/
OCIError     *errhp;  /* Error handle */
OCISvcCtx    *svchp;  /* Service Context handle*/

/*******************************************************************************
** Types and structures
*******************************************************************************/

struct point {
   double x;
   double y;
   double z;
};
typedef struct point point_struct;

struct geometry
{
    int gtype;
    int srid;
    struct point *point;
    int n_elem_info;
    int *elem_info;
    int n_ordinates;
    double *ordinates;
};
typedef struct geometry geometry_struct;

/*******************************************************************************
** Routine:     ReportError
**
** Description: Error message routine
*******************************************************************************/
void ReportError(OCIError *errhp)
{
  char errbuf[512];
  sb4 errcode = -1;

  OCIErrorGet(
    (dvoid *)errhp,                  /* (in)  Error handle */
    (ub4)1,                          /* (in)  Number of error record */
    (text *)NULL,                    /* (out) SQLSTATE (no longer used) */
    &errcode,                        /* (out) Error code */
    errbuf,                          /* (out) Buffer to receive error message */
    (ub4)sizeof(errbuf),             /* (in)  Size of error buffer */
    OCI_HTYPE_ERROR);                /* (in)  Type of handle (error) */

  fprintf(stderr, "ERROR %d: %s\n", errcode, errbuf);
  exit (1);
}

/*******************************************************************************
** Routine:     InitializeOCI
**
** Description: Initialize the OCI context
*******************************************************************************/
void InitializeOCI(void)
{
  /* Create and initialize OCI environment handle */
  OCIEnvCreate(
    &envhp,                          /* (out) Environment Handle */
    (ub4)(OCI_DEFAULT+OCI_OBJECT),   /* (in)  Mode: handles objects */
    (dvoid *)0,                      /* (in)  User defined context (NOT USED) */
    (dvoid *(*)())0,                 /* (in)  User-defined MALLOC routine (NOT USED) */
    (dvoid *(*)())0,                 /* (in)  User-defined REALLOC routine (NOT USED) */
    (void (*)())0,                   /* (in)  User-defined FREE routine (NOT USED) */
    (size_t)0,                       /* (in)  Size of extra user memory (NOT USED) */
    (dvoid **)0);                    /* (out) Pointer to user memory (NOT USED) */
  if (envhp == NULL) {
    printf ("OCIEnvCreate: failed to create environment handle\n");
    exit (1);
  }

  /* Allocate and initialize error report handle */
  OCIHandleAlloc(
    (dvoid *)envhp,                  /* (in)  Environment Handle */
    (dvoid **)&errhp,                /* (out) Error Handle */
    (ub4)OCI_HTYPE_ERROR,            /* (in)  Handle type (ERROR)*/
    (size_t)0,                       /* (in)  Size of extra user memory (NOT USED) */
    (dvoid **)0);                    /* (out) Pointer to user memory (NOT USED) */
  if (errhp == NULL) {
    printf ("OCIHandleAlloc: failed to create error handle\n");
    exit (1);
  }
}
/*******************************************************************************
** Routine:     ConnectDatabase
**
** Description: Connects to the oracle database
*******************************************************************************/
void ConnectDatabase(
        char *username,
        char *password,
        char *database)
{
  int status;
  char verbuf[512];

  /* Connect to database */
  status = OCILogon (
      envhp,                         /* (in)  Environment Handle */
      errhp,                         /* (in)  Error Handle */
      &svchp,                        /* (out) Service Context Handle */
      username, strlen(username),    /* (in)  Username */
      password, strlen(password),    /* (in)  Password */
      database, strlen(database));   /* (in)  Database (TNS service name) */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Get database version */
  OCIServerVersion(
    svchp,                           /* (in)  Service Context Handle */
    errhp,                           /* (in)  Error Handle */
    verbuf,                          /* (out) Buffer to receive version message */
    sizeof(verbuf),                  /* (in)  Size of message buffer */
    OCI_HTYPE_SVCCTX);               /* (in)  Type of handle (service context) */

  printf("Connected to: %s\n", database);
  printf("%s\n\n", verbuf);
}

/*******************************************************************************
** Routine:     DisconnectDatabase
**
** Description: Disconnect from Oracle
*******************************************************************************/
void DisconnectDatabase(void)
{
  int status;

  status = OCILogoff(svchp, errhp);
  if (status != OCI_SUCCESS)
    ReportError(errhp);
}

/*******************************************************************************
** Routine:     ClearOCI
**
** Description: Release the OCI context
*******************************************************************************/
void ClearOCI(void)
{

  /* Free error handle */
  OCIHandleFree(
    (dvoid *)errhp,                  /* (in)  Statement Handle */
    (ub4)OCI_HTYPE_ERROR);           /* (in)  Handle type */

  /* Terminate OCI context */
  OCITerminate (OCI_DEFAULT);
}

/*******************************************************************************
** Routine:     LoadGeometry
**
** Description: Load a geometry from an SDO_GEOMETRY object structure
**              into a C memory structure.
*******************************************************************************/
geometry_struct *LoadGeometry (
  SDO_GEOMETRY        *geometry_object,
  SDO_GEOMETRY_ind    *geometry_object_ind
)
{
  geometry_struct *geometry;
  double    x, y, z;
  boolean   exists;
  OCINumber *oci_number;
  long      i;
  static OCINumber *global_oci_number = NULL;
  static boolean   *global_exists = NULL;
  int status;

  if (geometry_object_ind->_atomic == OCI_IND_NULL) {
    geometry = NULL;
    return geometry;
  }

  /* Allocate geometry structure */
  geometry = malloc (sizeof(geometry_struct));

  /* Extract SDO_GTYPE */
  if (geometry_object_ind->SDO_GTYPE == OCI_IND_NOTNULL) {
    OCINumberToInt (
      errhp,
      &(geometry_object->SDO_GTYPE),
      (uword) sizeof (int),
      OCI_NUMBER_SIGNED,
      (dvoid *) & geometry->gtype);
  }
  else
    geometry->gtype = 0;

  /* Extract SDO_SRID */
  if (geometry_object_ind->SDO_SRID == OCI_IND_NOTNULL) {
    OCINumberToInt (
      errhp,
      &(geometry_object->SDO_SRID),
      (uword) sizeof (int),
      OCI_NUMBER_SIGNED,
      (dvoid *) & geometry->srid);
  }
  else
    geometry->srid = 0;

  /* Extract SDO_POINT */
  geometry->point = 0;
  if (geometry_object_ind->SDO_POINT._atomic == OCI_IND_NOTNULL) {
    x = y = z = 0;
    /* Allocate space for point structure */
    geometry->point = malloc (sizeof(point_struct));
    /* Extract X */
    if (geometry_object_ind->SDO_POINT.X == OCI_IND_NOTNULL)
      OCINumberToReal(
        errhp, &(geometry_object->SDO_POINT.X), (uword)sizeof(double), (dvoid *)&x);
    /* Extract Y */
    if (geometry_object_ind->SDO_POINT.Y == OCI_IND_NOTNULL)
      OCINumberToReal(
        errhp, &(geometry_object->SDO_POINT.Y), (uword)sizeof(double), (dvoid *)&y);
    /* Extract Z */
    if (geometry_object_ind->SDO_POINT.Z == OCI_IND_NOTNULL)
      OCINumberToReal(
        errhp, &(geometry_object->SDO_POINT.Z), (uword)sizeof(double), (dvoid *)&z);
    /* Fill point structure */
    geometry->point->x = x;
    geometry->point->y = y;
    geometry->point->z = z;
  }

  /* Extract SDO_ELEM_INFO array */

  /* Get the size of the array */
  OCICollSize (envhp, errhp,
    (OCIColl *)(geometry_object->SDO_ELEM_INFO), &geometry->n_elem_info);

  if (geometry->n_elem_info > 0) {

    /* Allocate memory for the array */
    geometry->elem_info = malloc (sizeof(int)*geometry->n_elem_info);

    /* Get all elements in the array */
    /* Loop over array elements and process one by one */
    for (i=0; i<geometry->n_elem_info; i++) {
      /* Extract one element from the varray */
      OCICollGetElem(envhp, errhp,
        (OCIColl *) (geometry_object->SDO_ELEM_INFO),
        (sb4)       (i),
        (boolean *) &exists,
        (dvoid **)  &oci_number,
        (dvoid **)  0
      );
      /* Convert the element to int */
      OCINumberToInt(errhp, oci_number,
        (uword)sizeof(int),
        OCI_NUMBER_UNSIGNED,
        (dvoid *)&geometry->elem_info[i]
      );
    }

  } else
   geometry->elem_info = NULL;

  /* Extract SDO_ORDINATES array */

  /* Get the size of the array */
  OCICollSize(envhp, errhp,
    (OCIColl *)(geometry_object->SDO_ORDINATES), &geometry->n_ordinates);

  if (geometry->n_ordinates > 0) {

    /* Allocate memory */
    geometry->ordinates = malloc (sizeof(double)*geometry->n_ordinates);

    /* Get all elements in the array */
    if (use_array_interface) {

      /* Use the collection array interface: process all elements at once */

      /* (Re)allocate space for intermediate vectors */
      global_oci_number = (OCINumber *)malloc(sizeof(OCINumber *) * geometry->n_ordinates);
      global_exists     = (boolean *)malloc(sizeof(boolean) * geometry->n_ordinates);

      /* Extract all elements from the varray */
      status = OCICollGetElemArray (envhp, errhp,
        (OCIColl *) geometry_object->SDO_ORDINATES,      /* Collection to process */
        (sb4) (0),                                       /* Index of first element to fetch */
        (boolean *) global_exists,                       /* Pointer to output array of presence flags */
        (dvoid **) &global_oci_number,                   /* Pointer to output array of OCINumber */
        (dvoid **) 0,                                    /* Pointer to output array of indicators (not used) */
        &(geometry->n_ordinates)                         /* Number of elements to extract */
      );
      if (status != OCI_SUCCESS)
       ReportError(errhp);

      for (i=0; i<geometry->n_ordinates; i++) {
        printf ("global_oci_number[%d]=%0X\n", i, global_oci_number[0]);
      }
      /* Convert all extracted elements to double */
      OCINumberToRealArray (errhp,
        (const OCINumber **) &global_oci_number,         /* Pointer to input array of OCINumber */
        geometry->n_ordinates,                           /* Number of elements to convert */
        (uword) sizeof (double),                         /* Size of output element */
        (dvoid *) geometry->ordinates);                  /* Pointer to output array of double */
    }
    else
    {
      /* Loop over array elements and process one by one */
      for (i=0; i<geometry->n_ordinates; i++) {

        /* extract one element from the varray */
        OCICollGetElem(envhp, errhp,
          (OCIColl *) (geometry_object->SDO_ORDINATES),
          (sb4)       (i),
          (boolean *) &exists,
          (dvoid **)  &oci_number,
          (dvoid **)  0
        );
        /* convert the element to double */
        OCINumberToReal(errhp, oci_number,
          (uword)sizeof(double),
          (dvoid *)&(geometry->ordinates[i])
        );
      }
    }

  } else
   geometry->ordinates = NULL;

  return (geometry);
}

/*******************************************************************************
** Routine:     FreeGeometry
**
** Description: Frees the memory used for a geometry structure
*******************************************************************************/
void FreeGeometry (geometry_struct *geometry) {
  if (geometry != 0) {
    /* Free memory used for the point structure */
    if (geometry->point != NULL)
      free (geometry->point);
    /* Free memory used for the elem_info vector */
    if (geometry->elem_info != 0)
      free (geometry->elem_info);
    /* Free memory used for the ordinates vector */
    if (geometry->ordinates != 0)
    free (geometry->ordinates);
    /* Free memory used for the geometry structure */
    free (geometry);
  }
}

/*******************************************************************************
** Routine:     PrintGeometry
**
** Description: Print out a geometry
*******************************************************************************/
void PrintGeometry (
  geometry_struct *geometry,
  int             row_number,
  int             print_level
)
{
  long i;
  int  gtype;
  char *gtype_name;
  int  dim;
  int  n_elements;
  int  n_points;

  gtype = geometry->gtype % 1000;
  dim = geometry->gtype / 1000;
  n_elements = geometry->n_elem_info / 3;
  n_points = geometry->n_ordinates / dim;
  switch (gtype) {
    case 1:
      gtype_name = "POINT";
      break;
    case 2:
      gtype_name = "LINESTRING";
      break;
    case 3:
      gtype_name = "POLYGON";
      break;
    case 4:
      gtype_name = "COLLECTION";
      break;
    case 5:
      gtype_name = "MULTI-POINT";
      break;
    case 6:
      gtype_name = "MULTI-LINESTRING";
      break;
    case 7:
      gtype_name = "MULTI-POLYGON";
      break;
  }

  if (print_level >= 1) {
    printf ("Row %d: ", row_number);
    printf ("Geometry\n");
    printf ("  Type: %d (%s)\n", gtype, gtype_name);
    printf ("  Dimensions: %d\n", dim);
    printf ("  Spatial reference system: %d\n", geometry->srid);
    printf ("  Elements: %d\n", n_elements);
    printf ("  Points: %d\n", n_points);
  }

  if (print_level >= 2) {
    printf ("Detailed structure\n");
    printf ("  SDO_GTYPE: %d\n", geometry->gtype);
    printf ("  SDO_SRID: %d\n", geometry->srid);
    if (geometry->point != NULL)
      printf ("  SDO_POINT: (%f, %f, %f)\n",
        geometry->point->x, geometry->point->y, geometry->point->z);
    if (geometry->n_elem_info > 0)
      printf ("  SDO_ELEM_INFO (%d elements)\n", geometry->n_elem_info);
    for (i=0; i<geometry->n_elem_info; i++)
      printf ("    [%d]=%d\n", i+1, geometry->elem_info[i]);
    if (geometry->n_ordinates > 0)
      printf ("  SDO_ORDINATES (%d elements)\n", geometry->n_ordinates);
    for (i=0; i<geometry->n_ordinates; i++)
      printf ("    [%d]=%f\n", i+1, geometry->ordinates[i]);
  }
}

/*******************************************************************************
** Routine:     ReadGeometries
**
** Description: Read all geometries returned by the select statement provided
*******************************************************************************/
void ReadGeometries (
  char *select_statement,
  int  print_level)
{
  int               rows_fetched = 0;        /* Row counter */
  OCIStmt           *select_stmthp;          /* Statement handle */
  sword             status;                  /* OCI call return status */

  /* Define handles for host variables */
  OCIDefine         *geometry_hp;

  /* Type descriptor for geometry object type */
  OCIType           *geometry_type_desc;

  /* Host variables */
  SDO_GEOMETRY      *geometry_obj = NULL;
  SDO_GEOMETRY_ind  *geometry_ind = NULL;

  geometry_struct   *geometry;

  /* Construct the select statement */
  printf ("Executing query:\nSQL> %s\n\n", select_statement);

  /* Initialize the statement handle */
  status = OCIHandleAlloc(
    (dvoid *)envhp,                  /* (in)  Environment Handle */
    (dvoid **)&select_stmthp,        /* (out) Statement Handle */
    (ub4)OCI_HTYPE_STMT,             /* (in)  Handle type*/
    (size_t)0,                       /* (in)  Size of extra user memory (NOT USED) */
    (dvoid **)0);                    /* (out) Pointer to user memory (NOT USED) */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Prepare the SQL statement  */
  status = OCIStmtPrepare(
    select_stmthp,                   /* (in)  Statement Handle */
    errhp,                           /* (in)  Error Handle */
    (text *)select_statement,        /* (in)  SQL statement */
    (ub4)strlen(select_statement),   /* (in)  Statement length */
    (ub4)OCI_NTV_SYNTAX,             /* (in)  Native SQL syntax */
    (ub4)OCI_DEFAULT);               /* (in)  Operating mode */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Get type descriptor for geometry object type */
  status = OCITypeByName (
    (dvoid *)envhp,                  /* (in)  Environment Handle */
    errhp,                           /* (in)  Error Handle */
    svchp,                           /* (in)  Service Context Handle */
    "MDSYS",                         /* (in)  Type owner name */
    strlen("MDSYS"),                 /* (in)  (length) */
    "SDO_GEOMETRY",                  /* (in)  Type name */
    strlen("SDO_GEOMETRY"),          /* (in)  (length) */
    0,                               /* (in)  Version name (NOT USED) */
    0,                               /* (in)  (length) */
    OCI_DURATION_SESSION,            /* (in)  Pin duration */
    OCI_TYPEGET_HEADER ,             /* (in)  Get option */
    &geometry_type_desc);            /* (out) Type descriptor */

  /* Define the variables to receive the selected columns */

  /* Variable 1 = geometry (ADT) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &geometry_hp,                    /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)1,                          /* (in)  Bind variable position */
    (dvoid *)0,                      /* (in)  Value Pointer (NOT USED) */
    0,                               /* (in)  Value Size (NOT USED) */
    SQLT_NTY,                        /* (in)  Data Type */
    (dvoid *)0,                      /* (in)  Indicator Pointer (NOT USED) */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED) */
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  status = OCIDefineObject(
    geometry_hp,                     /* (in)  Define handle */
    errhp,                           /* (in)  Error handle */
    geometry_type_desc,              /* (in)  Geometry type descriptor */
    (dvoid **) &geometry_obj,        /* (in)  Value Pointer */
    (ub4 *)0,                        /* (in)  Value Size (NOT USED) */
    (dvoid **) &geometry_ind,        /* (in)  Indicator Pointer */
    (ub4 *)0                         /* (in)  Indicator Size */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Execute query and fetch first row of result set */
  status = OCIStmtExecute(
    svchp,                           /* (in)  Service Context Handle */
    select_stmthp,                   /* (in)  Statement Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)1,                          /* (in)  Number of rows to fetch: 1 */
    (ub4)0,                          /* (in)  Row offset (NOT USED) */
    (OCISnapshot *)NULL,             /* (in)  Snapshot in (NOT USED) */
    (OCISnapshot *)NULL,             /* (in)  Snapshot out (NOT USED) */
    (ub4)OCI_DEFAULT);               /* (in)  Operating mode */
  if (status != OCI_SUCCESS && status != OCI_NO_DATA)
    ReportError(errhp);

  while (status != OCI_NO_DATA)
  {
    rows_fetched++;

    /* Import geometry from SDO_GEOMETRY OCI structure into C structure */
    geometry = LoadGeometry (geometry_obj, geometry_ind);

    /* Print the geometry just imported */
    PrintGeometry (geometry, rows_fetched, print_level);

    /* Release memory used for the geometry structure */
    FreeGeometry (geometry);

    /* Fetch next row of result set */
    status = OCIStmtFetch(
      select_stmthp,                 /* (in)  Statement Handle */
      errhp,                         /* (in)  Error Handle */
      (ub4)1,                        /* (in)  Number of rows to fetch */
      (ub2)OCI_FETCH_NEXT,           /* (in)  Fetch direction */
      (ub4)OCI_DEFAULT);             /* (in)  Operating mode */
    if (status != OCI_SUCCESS && status != OCI_NO_DATA)
      ReportError(errhp);
  }
  printf ("\n%d rows fetched\n", rows_fetched);

  /* Free statement handle */
  status = OCIHandleFree(
    (dvoid *)select_stmthp,          /* (in)  Statement Handle */
    (ub4)OCI_HTYPE_STMT);            /* (in)  Handle type */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

}

/*******************************************************************************
** Routine:     Main
**
** Description: Program main
*******************************************************************************/
int main(int argc, char **argv)
{
    char *username, *password, *database, *select_statement;
    int  print_level;

    if( argc != 6) {
      printf("USAGE: %s <username> <password> <database> <select_statement> <print_level>\n", argv[0]);
      exit( 1 );
    }
    else {
      username = argv[1];
      password = argv[2];
      database = argv[3];
      select_statement = argv[4];
      print_level = atoi (argv[5]);
    }

    /* Set up OCI environment */
    InitializeOCI();

    /* Connect to database */
    ConnectDatabase(username, password, database);

    /* Fetch and process the records */
    ReadGeometries(select_statement, print_level);

    /* disconnect from database */
    DisconnectDatabase();

    /* Teardown  OCI environment */
    ClearOCI();

}
/* read_points.c

   This program selects point objects from a table in the database.

   It reads the points without using object types, i.e by extracting
   the X and Y values of each point, using the following syntax:

   SELECT C.geo_column.SDO_POINT.X, C.geo_column.SDO_POINT.Y
     FROM tablename C

   It illustrates the following concepts:
   - dynamically constructing SQL statements
   - reading point details without using objects

   The program takes the following command line arguments:

     read_points username password database tablename geo_column

   where

   - username = name of the user to connect as
   - password = password for that user
   - database = TNS service name for the database
   - tablename = name of points table to select from
   - geo_column = name of the geometry column to read

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>

/*******************************************************************************
** Global variables
*******************************************************************************/

/* OCI handles */

OCIEnv       *envhp;  /* Environment handle*/
OCIError     *errhp;  /* Error handle */
OCISvcCtx    *svchp;  /* Service Context handle*/

/*******************************************************************************
** Routine:     ReportError
**
** Description: Error message routine
*******************************************************************************/
void ReportError(OCIError *errhp)
{
  char errbuf[512];
  sb4 errcode = 0;

  OCIErrorGet(
    (dvoid *)errhp,                    /* (in)  Error handle */
    (ub4)1,                            /* (in)  Number of error record */
    (text *)NULL,                      /* (out) SQLSTATE (no longer used) */
    &errcode,                          /* (out) Error code */
    errbuf,                            /* (out) Buffer to receive error message */
    (ub4)sizeof(errbuf),               /* (in)  Size of error buffer */
    OCI_HTYPE_ERROR);                  /* (in)  Type of handle (error) */

  fprintf(stderr, "%s\n", errbuf);
  exit (1);
}

/*******************************************************************************
** Routine:     InitializeOCI
**
** Description: Initialize the OCI context
*******************************************************************************/
void InitializeOCI(void)
{
  /* Create and initialize OCI environment handle */
  OCIEnvCreate(
    &envhp,                          /* (out) Environment Handle */
    (ub4)(OCI_DEFAULT),              /* (in)  Mode: default */
    (dvoid *)0,                      /* (in)  User defined context (NOT USED) */
    (dvoid *(*)())0,                 /* (in)  User-defined MALLOC routine (NOT USED) */
    (dvoid *(*)())0,                 /* (in)  User-defined REALLOC routine (NOT USED) */
    (void (*)())0,                   /* (in)  User-defined FREE routine (NOT USED) */
    (size_t)0,                       /* (in)  Size of extra user memory (NOT USED) */
    (dvoid **)0);                    /* (out) Pointer to user memory (NOT USED) */
  if (envhp == NULL) {
    printf ("OCIEnvCreate: failed to create environment handle\n");
    exit (1);
  }

  /* Allocate and initialize error report handle */
  OCIHandleAlloc(
    (dvoid *)envhp,                  /* (in)  Environment Handle */
    (dvoid **)&errhp,                /* (out) Error Handle */
    (ub4)OCI_HTYPE_ERROR,            /* (in)  Handle type (ERROR)*/
    (size_t)0,                       /* (in)  Size of extra user memory (NOT USED) */
    (dvoid **)0);                    /* (out) Pointer to user memory (NOT USED) */
  if (errhp == NULL) {
    printf ("OCIHandleAlloc: failed to create error handle\n");
    exit (1);
  }
}
/*******************************************************************************
** Routine:     ConnectDatabase
**
** Description: Connects to the oracle database
*******************************************************************************/
void ConnectDatabase(
        char *username,
        char *password,
        char *database)
{
  int status;
  char verbuf[512];

  /* Connect to database */
  status = OCILogon (
      envhp,                         /* (in)  Environment Handle */
      errhp,                         /* (in)  Error Handle */
      &svchp,                        /* (out) Service Context Handle */
      username, strlen(username),    /* (in)  Username */
      password, strlen(password),    /* (in)  Password */
      database, strlen(database));   /* (in)  Database (TNS service name) */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Get database version */
  OCIServerVersion(
    svchp,                             /* (in)  Service Context Handle */
    errhp,                             /* (in)  Error Handle */
    verbuf,                            /* (out) Buffer to receive version message */
    sizeof(verbuf),                    /* (in)  Size of message buffer */
    OCI_HTYPE_SVCCTX);                 /* (in)  Type of handle (service context) */

  printf("Connected to: %s\n", database);
  printf("%s\n\n", verbuf);
}

/*******************************************************************************
** Routine:     DisconnectDatabase
**
** Description: Disconnect from Oracle
*******************************************************************************/
void DisconnectDatabase(void)
{
  int status;

  status = OCILogoff(svchp, errhp);
  if (status != OCI_SUCCESS)
    ReportError(errhp);
}

/*******************************************************************************
** Routine:     ClearOCI
**
** Description: Release the OCI context
*******************************************************************************/
void ClearOCI(void)
{

  /* Free error handle */
  OCIHandleFree(
    (dvoid *)errhp,                  /* (in)  Statement Handle */
    (ub4)OCI_HTYPE_ERROR);           /* (in)  Handle type */

  /* Terminate OCI context */
  OCITerminate (OCI_DEFAULT);
}

/*******************************************************************************
** Routine:     ReadPoints
**
** Description: Read all points
*******************************************************************************/
void ReadPoints (
        char *tablename,
        char *geocolumn)
{
  int       rows_fetched = 0;        /* Row counter */
  char      select_sql[1024];        /* SQL Statement */
  OCIStmt   *select_stmthp;          /* Statement handle */
  sword     status;                  /* OCI call return status */

  /* Define handles for host variables */
  OCIDefine *point_x_hp;
  OCIDefine *point_y_hp;

  /* Host variables */
  double    point_x;
  double    point_y;

  /* Construct the select statement */
  sprintf (select_sql, "SELECT C.%s.SDO_POINT.X, C.%s.SDO_POINT.Y FROM %s C", geocolumn, geocolumn, tablename);
  printf ("Executing query:\nSQL> %s\n\n", select_sql);

  /* Initialize the statement handle */
  status = OCIHandleAlloc(
    (dvoid *)envhp,                  /* (in)  Environment Handle */
    (dvoid **)&select_stmthp,        /* (out) Statement Handle */
    (ub4)OCI_HTYPE_STMT,             /* (in)  Handle type*/
    (size_t)0,                       /* (in)  Size of extra user memory (NOT USED) */
    (dvoid **)0);                    /* (out) Pointer to user memory (NOT USED) */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Prepare the SQL statement  */
  status = OCIStmtPrepare(
    select_stmthp,                   /* (in)  Statement Handle */
    errhp,                           /* (in)  Error Handle */
    (text *)select_sql,              /* (in)  SQL statement */
    (ub4)strlen(select_sql),         /* (in)  Statement length */
    (ub4)OCI_NTV_SYNTAX,             /* (in)  Native SQL syntax */
    (ub4)OCI_DEFAULT);               /* (in)  Operating mode */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Define the variables to receive the selected columns */

  /* Variable 1 = POINT_X (float) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &point_x_hp,                     /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)1,                          /* (in)  Bind variable position */
    (dvoid *) &point_x,              /* (in)  Value Pointer */
    sizeof(point_x),                 /* (in)  Value Size */
    SQLT_FLT,                        /* (in)  Data Type */
    (dvoid *)0,                      /* (in)  Indicator Pointer */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED)*/
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Variable 2 = POINT_Y (float) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &point_y_hp,                     /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)2,                          /* (in)  Bind variable position */
    (dvoid *) &point_y,              /* (in)  Value Pointer */
    sizeof(point_y),                 /* (in)  Value Size */
    SQLT_FLT,                        /* (in)  Data Type */
    (dvoid *)0,                      /* (in)  Indicator Pointer */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED)*/
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Execute query and fetch first row of result set */
  status = OCIStmtExecute(
    svchp,                           /* (in)  Service Context Handle */
    select_stmthp,                   /* (in)  Statement Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)1,                          /* (in)  Number of rows to fetch: 1 */
    (ub4)0,                          /* (in)  Row offset (NOT USED) */
    (OCISnapshot *)NULL,             /* (in)  Snapshot in (NOT USED) */
    (OCISnapshot *)NULL,             /* (in)  Snapshot out (NOT USED) */
    (ub4)OCI_DEFAULT);               /* (in)  Operating mode */
  if (status != OCI_SUCCESS && status != OCI_NO_DATA)
    ReportError(errhp);

  while (status != OCI_NO_DATA)
  {
    /* Display results just fetched */
    rows_fetched++;
    printf ("%d: (%f, %f)\n", rows_fetched, point_x, point_y);

    /* Fetch next row of result set */
    status = OCIStmtFetch(
      select_stmthp,                 /* (in)  Statement Handle */
      errhp,                         /* (in)  Error Handle */
      (ub4)1,                        /* (in)  Number of rows to fetch */
      (ub2)OCI_FETCH_NEXT,           /* (in)  Fetch direction */
      (ub4)OCI_DEFAULT);             /* (in)  Operating mode */
    if (status != OCI_SUCCESS && status != OCI_NO_DATA)
      ReportError(errhp);
  }
  printf ("\n%d rows fetched\n", rows_fetched);

  /* Free statement handle */
  status = OCIHandleFree(
    (dvoid *)select_stmthp,          /* (in)  Statement Handle */
    (ub4)OCI_HTYPE_STMT);            /* (in)  Handle type */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

}

/*******************************************************************************
** Routine:     Main
**
** Description: Program main
*******************************************************************************/
int main(int argc, char **argv)
{
    char *username, *password, *database, *tablename, *geocolumn;

    if( argc != 6) {
      printf("USAGE: %s <username> <password> <database> <tablename> <geo_column>\n", argv[0]);
      exit( 1 );
    }
    else {
      username = argv[1];
      password = argv[2];
      database = argv[3];
      tablename = argv[4];
      geocolumn = argv[5];
    }

    /* Set up OCI environment */
    InitializeOCI();

    /* Connect to database */
    ConnectDatabase(username, password, database);

    /* Fetch and process the records */
    ReadPoints(tablename, geocolumn);

    /* disconnect from database */
    DisconnectDatabase();

    /* Teardown  OCI environment */
    ClearOCI();

}
