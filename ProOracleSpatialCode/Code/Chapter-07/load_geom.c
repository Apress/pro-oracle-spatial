/* load_geom.c

   This program loads geometry objects into a table in the database.

   The geometries are read from a text file. Each record in this file
   contains information about one geometry, using the following format:

     id type dim x1 y1 x2 y2 ... xn yn

   where

   - id = the numeric identifier of the geometry
   - type = the type of geometry: 1, 2 or 3
   - dim = the number of dimensions
   - xi, yi = the X and Y of the points that form the geometry

   The geometry objects can be of any kind. Each object is first
   loaded into a C structure, from which it is stored into the
   SDO_GEOMETRY OCI structure.

   The program illustrates the following concepts:
   - encoding and writing geometry objects

   The program takes the following command line arguments:

     load_geom username password database table id_column geo_column filename

   where

   - username = name of the user to connect as
   - password = password for that user
   - database = TNS service name for the database
   - tablename = the table to load into
   - id_column = the id column
   - geo_column = the geometry column to load into
   - filename = the input file to process

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
** Routine:     StoreGeometry
**
** Description: Stores a geometry from a C memory structure.into an SDO_GEOMETRY
**              object structure
*******************************************************************************/
SDO_GEOMETRY *StoreGeometry ()
{
}

/*******************************************************************************
** Routine:     ReadGeometryFromFile
**
** Description: Reads and parses a geometry from the input file
*******************************************************************************/
geometry_struct* ReadGeometryFromFile (
  FILE *input_file
  )
{

}

/*******************************************************************************
** Routine:     LoadGeometries
**
** Description: Load all geometries from the input file
*******************************************************************************/
void LoadGeometries (
  char *tablename,
  char *id_column,
  char *geo_column,
  char *filename)
{
  int               rows_loaded = 0;         /* Row counter */
  char              insert_statement[1024];  /* Buffer to build INSERT statement */
  OCIStmt           *insert_stmthp;          /* Statement handle */
  sword             status;                  /* OCI call return status */
  FILE              *input_file;

  /* Bind handles for input variables */
  OCIBind           *id_hp = NULL;
  OCIBind           *geometry_hp = NULL;

  /* Input variables */
  long      id;
  SDO_GEOMETRY      *geometry_obj = NULL;
  SDO_GEOMETRY_ind  *geometry_ind = NULL;

  /* Type descriptor for geometry object type */
  OCIType           *geometry_type_desc;

  /* Host variables */
  geometry_struct   *geometry;

  /* Open input file */
  input_file = fopen (filename, "r");
  if (input_file == NULL) {
    printf ("Could not open file %s\n", filename);
    exit (1);
  }

  /* Construct the insert statement */
  sprintf (insert_statement, "insert into %s (%s, %s) values (:id, :geometry)", tablename, id_column, geo_column);
  printf ("Executing :\nSQL> %s\n\n", insert_statement);

  /* Initialize the statement handle */
  status = OCIHandleAlloc(
    (dvoid *)envhp,                  /* (in)  Environment Handle */
    (dvoid **)&insert_stmthp,        /* (out) Statement Handle */
    (ub4)OCI_HTYPE_STMT,             /* (in)  Handle type*/
    (size_t)0,                       /* (in)  Size of extra user memory (NOT USED) */
    (dvoid **)0);                    /* (out) Pointer to user memory (NOT USED) */
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Prepare the SQL statement  */
  status = OCIStmtPrepare(
    insert_stmthp,                   /* (in)  Statement Handle */
    errhp,                           /* (in)  Error Handle */
    (text *)insert_statement,        /* (in)  SQL statement */
    (ub4)strlen(insert_statement),   /* (in)  Statement length */
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

  /* Bind the input variables */

  /* ID (integer) */
  status = OCIBindByName(
    insert_stmthp,                   /* (in)  Statement Handle */
    &id_hp,                          /* (out) Bind Handle */
    errhp,                           /* (in)  Error Handle */
    (text *) ":ID",                  /* (in)  Placeholder */
    strlen(":ID"),                   /* (in)  Placeholder length */
    (ub1 *) &id,                     /* (in)  Value Pointer */
    sizeof(id),                      /* (in)  Value Size */
    SQLT_INT,                        /* (in)  Data Type */
    (dvoid *) 0,                     /* (in)  Indicator Pointer (NOT USED) */
    (ub2 *) 0,                       /* (out) Actual length (NOT USED) */
    (ub2) 0,                         /* (out) Column return codes (NOT USED) */
    (ub4) 0,                         /* (in)  (NOT USED) */
    (ub4 *) 0,                       /* (in)  (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* GEOMETRY (ADT) */
  status = OCIBindByName(
    insert_stmthp,                   /* (in)  Statement Handle */
    &geometry_hp,                    /* (out) Bind Handle */
    errhp,                           /* (in)  Error Handle */
    (text *) ":GEOMETRY",            /* (in)  Placeholder */
    strlen(":GEOMETRY"),             /* (in)  Placeholder length */
    (ub1 *) 0,                       /* (in)  Value Pointer (NOT USED) */
    0,                               /* (in)  Value Size (NOT USED) */
    SQLT_NTY,                        /* (in)  Data Type */
    (dvoid *) 0,                     /* (in)  Indicator Pointer (NOT USED) */
    (ub2 *) 0,                       /* (out) Actual length (NOT USED) */
    (ub2) 0,                         /* (out) Column return codes (NOT USED) */
    (ub4) 0,                         /* (in)  (NOT USED) */
    (ub4 *) 0,                       /* (in)  (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  status = OCIBindObject(
    geometry_hp,                     /* (in)  Bind handle */
    errhp,                           /* (in)  Error handle */
    geometry_type_desc,              /* (in)  Geometry type descriptor */
    (dvoid **) &geometry_obj,        /* (in)  Value Pointer */
    (ub4 *)0,                        /* (in)  Value Size (NOT USED) */
    (dvoid **) &geometry_ind,        /* (in)  Indicator Pointer */
    (ub4 *)0                         /* (in)  Indicator Size */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  geometry = ReadGeometryFromFile(input_file);
  while (geometry != NULL)
  {
    rows_loaded++;

    /* Store geometry from C structure into SDO_GEOMETRY OCI structure*/
    StoreGeometry (geometry, geometry_obj, geometry_ind);

    /* Execute insert statement */
    status = OCIStmtExecute(
      svchp,                         /* (in)  Service Context Handle */
      insert_stmthp,                 /* (in)  Statement Handle */
      errhp,                         /* (in)  Error Handle */
      (ub4)1,                        /* (in)  Number of rows to fetch: 1 */
      (ub4)0,                        /* (in)  Row offset (NOT USED) */
      (OCISnapshot *)NULL,           /* (in)  Snapshot in (NOT USED) */
      (OCISnapshot *)NULL,           /* (in)  Snapshot out (NOT USED) */
      (ub4)OCI_DEFAULT);             /* (in)  Operating mode */
    if (status != OCI_SUCCESS)
      ReportError(errhp);
    /* Read next geometry */
    geometry = ReadGeometryFromFile(input_file);
  }
  printf ("\n%d rows loaded\n", rows_loaded);

  /* Free statement handle */
  status = OCIHandleFree(
    (dvoid *)insert_stmthp,          /* (in)  Statement Handle */
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
    char *username, *password, *database, *tablename, *id_column, *geo_column, *filename;

    if( argc != 8) {
      printf("USAGE: %s <username> <password> <database> <tablename> <id_column> <geo_column> <filename>\n", argv[0]);
      exit( 1 );
    }
    else {
      username = argv[1];
      password = argv[2];
      database = argv[3];
      tablename = argv[4];
      id_column = argv[5];
      geo_column = argv[6];
      filename = argv[7];
    }

    /* Set up OCI environment */
    InitializeOCI();

    /* Connect to database */
    ConnectDatabase(username, password, database);

    /* Fetch and process the records */
    LoadGeometries(tablename, id_column, geo_column, filename);

    /* disconnect from database */
    DisconnectDatabase();

    /* Teardown  OCI environment */
    ClearOCI();

}
