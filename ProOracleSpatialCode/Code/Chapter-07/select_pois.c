/* select_pois.c

   This program selects all POIs from the US_POIS table that lie within a
   chosen distance from a given point. It returns the id, name and telephone
   number of each POI.

   It illustrates the following concepts:

   - using null indicators for the columns returned by the select statement
   - using bind variables to pass variable information to the select statement

   The program uses the following command line arguments:

     select_pois username password database poi_type x y distance unit

   where

   - username = name of the user to connect as
   - password = password for that user
   - database = TNS service name for the database
   - poi_type = type of POI to select (from FACILITY_NAME column)
   - X = longitude of search point
   - Y = latitude of search point
   - distance = distance to search
   - unit = name of distance unit (M, KM, MILES, etc)

   Notes:

   The coordinates of the search point are assumed to be in
   latitude/longitude WGS84 (srid 8307).

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>

/*******************************************************************************
** Constants
*******************************************************************************/
#define POI_NAME_LENGTH 35
#define PHONE_NUMBER_LENGTH 15
#define TRACE() {printf ("TRACE: %d\n", __LINE__);}

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
** Routine:     ReadPois
**
** Description: Read the POIs
*******************************************************************************/
void ReadPois (
  char   *poi_type,
  double x,
  double y,
  double distance,
  char   *unit)
{
  int       rows_fetched = 0;        /* Row counter */
  char      *select_sql =
    "SELECT id, poi_name, phone_number, "
    "sdo_geom.sdo_distance (location, sdo_geometry(2001, 8307, sdo_point_type(:x, :y, null), null, null), 1) distance "
    "from us_pois "
    "where facility_name = :poi_type "
    "and sdo_within_distance (location, sdo_geometry(2001, 8307, sdo_point_type(:x, :y, null), null, null), :distance_spec) = 'TRUE' "
    "order by distance";
  char      distance_spec[128];      /* Hold the distance specification string (DISTANCE=d UNIT=u) */
  OCIStmt   *select_stmthp;          /* Statement handle */
  sword     status;                  /* OCI call return status */

  /* Define handles for output variables */
  OCIDefine *id_hp;
  OCIDefine *poi_name_hp;
  OCIDefine *phone_number_hp;

  /* Bind handles for input variables */
  OCIBind   *poi_type_hp = NULL;
  OCIBind   *x_hp = NULL;
  OCIBind   *y_hp = NULL;
  OCIBind   *distance_spec_hp = NULL;

  /* Output variables */
  long      id;
  char      poi_name[POI_NAME_LENGTH+1];
  char      phone_number[PHONE_NUMBER_LENGTH+1];

  /* NULL indicators for output variables */
  sb2       id_ind;
  sb2       poi_name_ind;
  sb2       phone_number_ind;
  /* NOTE: If no null indicator is provided for a column and a null value is returned then
     the query fails with: ORA-01405: fetched column value is NULL  */

  /* Display the select statement */
  printf ("Executing query:\nSQL> %s\n\n", select_sql);

  /* Construct distance specifier for within_distance operator */
  sprintf (distance_spec, "distance=%f unit=%s", distance, unit);

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

  /* Bind the input variables */

  /* POI_TYPE (string) */
  status = OCIBindByName(
    select_stmthp,                   /* (in)  Statement Handle */
    &poi_type_hp,                    /* (out) Bind Handle */
    errhp,                           /* (in)  Error Handle */
    (text *) ":POI_TYPE",            /* (in)  Placeholder */
    strlen(":POI_TYPE"),             /* (in)  Placeholder length */
    (ub1 *) poi_type,                /* (in)  Value Pointer */
    strlen(poi_type)+1,              /* (in)  Value Size */
    SQLT_STR,                        /* (in)  Data Type */
    (dvoid *) 0,                     /* (in)  Indicator Pointer (NOT USED) */
    (ub2 *) 0,                       /* (out) Actual length (NOT USED) */
    (ub2) 0,                         /* (out) Column return codes (NOT USED) */
    (ub4) 0,                         /* (in)  (NOT USED) */
    (ub4 *) 0,                       /* (in)  (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* X (float) */
  status = OCIBindByName(
    select_stmthp,                   /* (in)  Statement Handle */
    &x_hp,                           /* (out) Bind Handle */
    errhp,                           /* (in)  Error Handle */
    (text *) ":X",                   /* (in)  Placeholder */
    strlen(":X"),                    /* (in)  Placeholder length */
    (ub1 *) &x,                      /* (in)  Value Pointer */
    sizeof(x),                       /* (in)  Value Size */
    SQLT_FLT,                        /* (in)  Data Type */
    (dvoid *) 0,                     /* (in)  Indicator Pointer (NOT USED) */
    (ub2 *) 0,                       /* (out) Actual length (NOT USED) */
    (ub2) 0,                         /* (out) Column return codes (NOT USED) */
    (ub4) 0,                         /* (in)  (NOT USED) */
    (ub4 *) 0,                       /* (in)  (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Y (float) */
  status = OCIBindByName(
    select_stmthp,                   /* (in)  Statement Handle */
    &y_hp,                           /* (out) Bind Handle */
    errhp,                           /* (in)  Error Handle */
    (text *) ":Y",                   /* (in)  Placeholder */
    strlen(":Y"),                    /* (in)  Placeholder length */
    (ub1 *) &y,                      /* (in)  Value Pointer */
    sizeof(y),                       /* (in)  Value Size */
    SQLT_FLT,                        /* (in)  Data Type */
    (dvoid *) 0,                     /* (in)  Indicator Pointer (NOT USED) */
    (ub2 *) 0,                       /* (out) Actual length (NOT USED) */
    (ub2) 0,                         /* (out) Column return codes (NOT USED) */
    (ub4) 0,                         /* (in)  (NOT USED) */
    (ub4 *) 0,                       /* (in)  (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* DISTANCE_SPEC (string) */
  status = OCIBindByName(
    select_stmthp,                   /* (in)  Statement Handle */
    &distance_spec_hp,               /* (out) Bind Handle */
    errhp,                           /* (in)  Error Handle */
    (text *) ":DISTANCE_SPEC",       /* (in)  Placeholder */
    strlen(":DISTANCE_SPEC"),        /* (in)  Placeholder length */
    (ub1 *) distance_spec,           /* (in)  Value Pointer */
    sizeof(distance_spec),           /* (in)  Value Size */
    SQLT_STR,                        /* (in)  Data Type */
    (dvoid *) 0,                     /* (in)  Indicator Pointer (NOT USED) */
    (ub2 *) 0,                       /* (out) Actual length (NOT USED) */
    (ub2) 0,                         /* (out) Column return codes (NOT USED) */
    (ub4) 0,                         /* (in)  (NOT USED) */
    (ub4 *) 0,                       /* (in)  (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Define the output variables to receive the selected columns */

  /* Variable 1 = ID (integer) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &id_hp,                          /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)1,                          /* (in)  Bind variable position */
    (dvoid *) &id,                   /* (in)  Value Pointer */
    sizeof(id),                      /* (in)  Value Size */
    SQLT_INT,                        /* (in)  Data Type */
    (dvoid *) &id_ind,               /* (in)  Indicator Pointer */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED)*/
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Variable 2 = POI_NAME (string) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &poi_name_hp,                    /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)2,                          /* (in)  Bind variable position */
    (dvoid *) &poi_name,             /* (in)  Value Pointer */
    sizeof(poi_name),                /* (in)  Value Size */
    SQLT_STR,                        /* (in)  Data Type */
    (dvoid *) &poi_name_ind,         /* (in)  Indicator Pointer */
    (ub2 *)0,                        /* (out) Length of data fetched (NOT USED)*/
    (ub2 *)0,                        /* (out) Column return codes (NOT USED) */
    (ub4)OCI_DEFAULT                 /* (in)  Operating mode */
  );
  if (status != OCI_SUCCESS)
    ReportError(errhp);

  /* Variable 3 = PHONE_NUMBER (string) */
  status = OCIDefineByPos(
    select_stmthp,                   /* (in)  Statement Handle */
    &phone_number_hp,                /* (out) Define Handle */
    errhp,                           /* (in)  Error Handle */
    (ub4)3,                          /* (in)  Bind variable position */
    (dvoid *) &phone_number,         /* (in)  Value Pointer */
    sizeof(phone_number),            /* (in)  Value Size */
    SQLT_STR,                        /* (in)  Data Type */
    (dvoid *) &phone_number_ind,     /* (in)  Indicator Pointer */
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

    /* Consider that PHONE_NUMBER could be null.*/
    if (phone_number_ind == OCI_IND_NULL)
      strcpy ( phone_number, "NO TELEPHONE");

    printf ("%d: %d %*s %s\n", rows_fetched, id, POI_NAME_LENGTH, poi_name, phone_number);

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
    char *username, *password, *database, *poi_type, *unit;
    double x, y, distance;
    int i;

    if( argc != 9) {
      printf("USAGE: %s <username> <password> <database> <poi_type> <x> <y> <distance> <unit>\n", argv[0]);
      exit( 1 );
    }
    else {
      username = argv[1];
      password = argv[2];
      database = argv[3];
      poi_type = argv[4];
      sscanf(argv[5], "%lf", &x);
      sscanf(argv[6], "%lf", &y);
      sscanf(argv[7], "%lf", &distance);
      unit = argv[8];
    }

    /* Set up OCI environment */
    InitializeOCI();

    /* Connect to database */
    ConnectDatabase(username, password, database);

    /* Fetch and process the records */
    ReadPois(poi_type, x, y, distance, unit);

    /* disconnect from database */
    DisconnectDatabase();

    /* Teardown  OCI environment */
    ClearOCI();

}
