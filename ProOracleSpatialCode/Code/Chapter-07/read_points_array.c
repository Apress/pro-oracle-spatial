/* read_points_array.c

   This program selects point objects from a table in the database.

   It reads the points without using object types, i.e by extracting
   the X and Y values of each point, using the following syntax:

   SELECT C.geo_column.SDO_POINT.X, C.geo_column.SDO_POINT.Y
     FROM tablename C

   It is identical to read_points.c except that it fetches multiple rows
   at a time (array fetches).

   It illustrates the following concepts:
   - dynamically constructing SQL statements
   - reading point details without using objects
   - using array fetches

   The program takes the following command line arguments:

     read_points username password database tablename geo_column [array_size]

   where

   - username = name of the user to connect as
   - password = password for that user
   - database = TNS service name for the database
   - tablename = name of points table to select from
   - geo_column = name of the geometry column to read
   - array_size = number of rows to read per fetch (default is 10 rows)

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
  char *geocolumn,
  int  array_size)
{
  int       rows_fetched = 0;        /* Row counter */
  int       nr_fetches = 0;          /* Number of batches fetched */
  int       rows_in_batch = 0;       /* Number of rows in current batch */
  boolean   has_more_data;
  char      select_sql[1024];        /* SQL Statement */
  OCIStmt   *select_stmthp;          /* Statement handle */
  sword     status;                  /* OCI call return status */
  int       i, j, k;

  /* Define handles for host variables */
  OCIDefine *point_x_hp;
  OCIDefine *point_y_hp;

  /* Host variables */
  double    point_x[array_size];
  double    point_y[array_size];

  /* Construct the select statement */
  sprintf (select_sql, "SELECT C.%s.SDO_POINT.X, C.%s.SDO_POINT.Y FROM %s C", geocolumn, geocolumn, tablename);
  printf ("Executing query:\nSQL> %s\n\n", select_sql);

  /* Initialize the statement handle */
  status = OCIHandleAlloc(
    (dvoid *)envhp,                  /* (in)  Environment Handle */
    (dvoid **)&select_stmthp,        /* (out) Statement Handle */
    (ub4)OCI_HTYPE_STMT,             /* (in)  Handle type */
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
    sizeof(double),                  /* (in)  Value Size */
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
    sizeof(double),                  /* (in)  Value Size */
    SQLT_FLT,                        /* (in)  Data Type */
    (dvoid *)0,                      /* (in)  Indicator Pointer (NOT USED) */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED) */
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
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
      printf ("%d: (%f, %f)\n", rows_fetched, point_x[i], point_y[i]);
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
    char *username, *password, *database, *tablename, *geocolumn;
    int array_size;

    if( argc < 6 || argc > 7) {
      printf("USAGE: %s <username> <password> <database> <tablename> <geo_column> [<array_size>]\n", argv[0]);
      exit( 1 );
    }
    else {
      username = argv[1];
      password = argv[2];
      database = argv[3];
      tablename = argv[4];
      geocolumn = argv[5];
      if (argc > 6)
        array_size = atoi(argv[6]);
      else
        array_size = 10;
      if (array_size <= 0) {
        printf ("Invalid array size: must be positive\n");
        exit( 1 );
      }
    }

    /* Set up OCI environment */
    InitializeOCI();

    /* Connect to database */
    ConnectDatabase(username, password, database);

    /* Fetch and process the records */
    ReadPoints(tablename, geocolumn, array_size);

    /* disconnect from database */
    DisconnectDatabase();

    /* Teardown  OCI environment */
    ClearOCI();

}
