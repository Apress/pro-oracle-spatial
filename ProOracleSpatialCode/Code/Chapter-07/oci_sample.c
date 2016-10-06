/* oci_sample.c

   This program illustrates the basic operation of an OCI program.
   It executes the following simple SQL statement to read information from
   the US_CITIES table:

   SELECT ID, CITY, STATE_ABRV, POP90, RANK90 FROM US_CITIES;

   It illustrates the following concepts:
   - setting up and tearing down the OCI environment
   - connecting to and disconnecting from an Oracle database
   - executing a SELECT statement
   - using host ("defined") variables to read the results of a SELECT statement.

   The program takes the following command line arguments:

     oci_sample username password database

   where

   - username = name of the user to connect as
   - password = password for that user
   - database = TNS service name for the database

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>

/*******************************************************************************
** Constants
*******************************************************************************/
#define CITY_LENGTH 42
#define STATE_ABRV_LENGTH 2

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
  OCIEnvCreate(&envhp, OCI_DEFAULT+OCI_OBJECT, 0,0,0,0,0,0);
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
  OCIHandleAlloc(envhp, &errhp, OCI_HTYPE_ERROR, 0, 0);
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
** Routine:     ReadCities
**
** Description: Read all cities
*******************************************************************************/
void ReadCities (void)
{
  int       rows_fetched = 0;        /* Row counter */
  char      *select_sql;             /* SQL Statement */
  OCIStmt   *select_stmthp;          /* Statement handle */
  sword     status;                  /* OCI call return status */

  /* Define handles for host variables */
  OCIDefine *id_hp;
  OCIDefine *city_hp;
  OCIDefine *state_abrv_hp;
  OCIDefine *pop90_hp;
  OCIDefine *rank90_hp;

  /* Host variables */
  int       id;
  char      city[CITY_LENGTH+1];
  char      state_abrv[STATE_ABRV_LENGTH+1];
  double    pop90;
  long      rank90;

  /* Construct the select statement */
  select_sql = "SELECT ID, CITY, STATE_ABRV, POP90, RANK90 FROM US_CITIES";
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

  /* Variable 1 = ID (integer) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &id_hp,                          /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)1,                          /* (in)  Bind variable position */
    (dvoid *) &id,                   /* (in)  Value Pointer */
    sizeof(id),                      /* (in)  Value Size */
    SQLT_INT,                        /* (in)  Data Type */
    (dvoid *)0,                      /* (in)  Indicator Pointer */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED)*/
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Variable 2 = CITY (string) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &city_hp           ,             /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)2,                          /* (in)  Bind variable position */
    city,                            /* (in)  Value Pointer */
    sizeof(city),                    /* (in)  Value Size */
    SQLT_STR,                        /* (in)  Data Type */
    (dvoid *)0,                      /* (in)  Indicator Pointer */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED)*/
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Variable 3 = STATE_ABRV (string) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &state_abrv_hp,                  /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)3,                          /* (in)  Bind variable position */
    state_abrv,                      /* (in)  Value Pointer */
    sizeof(state_abrv),              /* (in)  Value Size */
    SQLT_STR,                        /* (in)  Data Type */
    (dvoid *)0,                      /* (in)  Indicator Pointer */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED)*/
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Variable 4 = POP90 (float) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &pop90_hp,                       /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)4,                          /* (in)  Bind variable position */
    (dvoid *) &pop90,                /* (in)  Value Pointer */
    sizeof(pop90),                   /* (in)  Value Size */
    SQLT_FLT,                        /* (in)  Data Type */
    (dvoid *)0,                      /* (in)  Indicator Pointer */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED)*/
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Variable 5 = RANK90 (integer) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &rank90_hp,                      /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)5,                          /* (in)  Bind variable position */
    (dvoid *) &rank90,               /* (in)  Value Pointer */
    sizeof(rank90),                  /* (in)  Value Size */
    SQLT_INT,                        /* (in)  Data Type */
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
    printf ("Row %d: %d %s %s %f %d\n", rows_fetched, id, city, state_abrv, pop90, rank90);

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
    char *username, *password, *database;

    if( argc < 3 || argc > 4) {
      printf("USAGE: %s <username> <password> [<database>] \n", argv[0]);
      exit( 1 );
    }
    else {
      username = argv[1];
      password = argv[2];
      if (argc > 3)
        database = argv[3];
      else
        database = "";
    }

    /* Set up OCI environment */
    InitializeOCI();

    /* Connect to database */
    ConnectDatabase(username, password, database);

    /* Fetch and process the records */
    ReadCities();

    /* disconnect from database */
    DisconnectDatabase();

    /* Teardown  OCI environment */
    ClearOCI();

}
