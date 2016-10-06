REM Listing 5-14. Enabling Spatial Indexes for the Tables in the Transported Tablespace
SQLPLUS SYS/CHANGE_ON_INSTALL AS SYSDBA
ALTER TABLESPACE TBS READ WRITE;
CONNECT spatial/spatial;
EXEC SDO_UTIL.INITIALIZE_INDEXES_FOR_TTS;
