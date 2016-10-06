/* read_geom_array.c

   This program reads geometry objects from a table in the database.

   The geometry objects can be of any kind. Each object is first
   loaded into a C structure, from which it is then printed out.

   The input to the program is a SELECT statement that returns a single
   column of type SDO_GEOMETRY.

   It is identical to read_geom.c except that it fetches multiple rows
   at a time (array fetches).

   It illustrates the following concepts:
   - passing SQL statements from the command line
   - reading and decoding geometry objects
   - using array fetches

   The program takes the following command line arguments:

     read_geom username password database select_statement print_level [array_size]

   where

   - username = name of the user to connect as
   - password = password for that user
   - database = TNS service name for the database
   - print_level = level of detail to print out
     0 = do not print geometries
     1 = print summary (type, number of elements, number of points)
     2 = print details (all elements and point details)
   - array_size = number of rows to read per fetch (default is 10 rows)

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oci.h>
#include "sdo_geometry.h"

#define use_array_interface 0
#define TRACE() {printf ("TRACE: %d\n", __LINE__);}

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
  int  print_level,
  int  array_size)
{
  int       rows_fetched = 0;        /* Row counter */
  int       nr_fetches = 0;          /* Number of batches fetched */
  int       rows_in_batch = 0;       /* Number of rows in current batch */
  boolean   has_more_data;
  OCIStmt   *select_stmthp;          /* Statement handle */
  sword     status;                  /* OCI call return status */
  int       i, j, k;

  /* Define handles for host variables */
  OCIDefine         *geometry_hp;

  /* Type descriptor for geometry object type */
  OCIType           *geometry_type_desc;

  /* Host variables */
  SDO_GEOMETRY      *geometry_obj[array_size];
  SDO_GEOMETRY_ind  *geometry_ind[array_size];

  geometry_struct   *geometry;

  /* Construct the select statement */
  printf ("Executing query:\nSQL> %s\n", select_statement);
  printf ("Array size: %d\n\n", array_size);

  /* Initialize array of geometry pointers */
  for (i=0; i<array_size; i++) {
    geometry_obj[i] = NULL;
    geometry_ind[i] = NULL;
  }

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

  /* Execute query and fetch first batch of rows of result set */
  status = OCIStmtExecute(
    svchp,                           /* (in)  Service Context Handle */
    select_stmthp,                   /* (in)  Statement Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)array_size,                 /* (in)  Number of rows to fetch */
    (ub4)0,                          /* (in)  Row offset (NOT USED) */
    (OCISnapshot *)NULL,             /* (in)  Snapshot in (NOT USED) */
    (OCISnapshot *)NULL,             /* (in)  Snapshot out (NOT USED) */
    (ub4)OCI_DEFAULT);               /* (in)  Operating mode */
  if (status != OCI_SUCCESS && status != OCI_NO_DATA)
    ReportError(errhp);

  has_more_data = TRUE;
  do
  {
    /* Check if this is the last (or only) batch.
       The Execute and Fetch calls return OCI_NO_DATA to indicate that the batch
       they returned is the last one, i.e. that no further calls to OCIStmtFetch are needed.
       The returned batch still needs to be processed */
    if (status == OCI_NO_DATA)
      has_more_data = FALSE;

    /* Get the number of rows returned in current batch */
    OCIAttrGet(
      (dvoid *)select_stmthp,
      (ub4)OCI_HTYPE_STMT,
      (dvoid *)&rows_in_batch,
      (ub4 *)0,
      (ub4)OCI_ATTR_ROWS_FETCHED,
      errhp);

    nr_fetches++;

    /* Display results just fetched */
    for (i=0; i<rows_in_batch; i++) {
      rows_fetched++;

      /* Import geometry from SDO_GEOMETRY OCI structure into C structure */
      geometry = LoadGeometry (geometry_obj[i], geometry_ind[i]);

      /* Print the geometry just imported */
      PrintGeometry (geometry, rows_fetched, print_level);

      /* Release memory used for the geometry structure */
      FreeGeometry (geometry);
    }

    if (has_more_data) {
      /* Fetch next batch of rows of result set */
      status = OCIStmtFetch(
        select_stmthp,                 /* (in)  Statement Handle */
        errhp,                         /* (in)  Error Handle */
        (ub4)array_size,                  /* (in)  Number of rows to fetch */
        (ub2)OCI_FETCH_NEXT,           /* (in)  Fetch direction */
        (ub4)OCI_DEFAULT);             /* (in)  Operating mode */
      if (status != OCI_SUCCESS && status != OCI_NO_DATA)
        ReportError(errhp);
    }
  }
  while (has_more_data);

  printf ("\n%d rows fetched in %d fetches\n", rows_fetched, nr_fetches);

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
    int  print_level, array_size;
    clock_t  start_time, end_time;

    if( argc < 6 || argc > 7) {
      printf("USAGE: %s <username> <password> <database> <select_statement> <print_level> [<array_size>]\n", argv[0]);
      exit( 1 );
    }
    else {
      username = argv[1];
      password = argv[2];
      database = argv[3];
      select_statement = argv[4];
      print_level = atoi (argv[5]);
      if (argc > 6)
        array_size = atoi(argv[6]);
      else
        array_size = 10;
      if (array_size <= 0) {
        printf ("Invalid array size: must be positive\n");
        exit( 1 );
      }
    }

    start_time = clock();

    /* Set up OCI environment */
    InitializeOCI();

    /* Connect to database */
    ConnectDatabase(username, password, database);

    /* Fetch and process the records */
    ReadGeometries(select_statement, print_level, array_size);

    /* disconnect from database */
    DisconnectDatabase();

    /* Teardown  OCI environment */
    ClearOCI();

    end_time = clock();
    printf("Elapsed time: %.3f seconds\n", (double) (end_time - start_time)/CLOCKS_PER_SEC);
}
