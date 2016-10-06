REM Listing 5-12. Transporting the Tablespace TBS from a Source Database
SQLPLUS spatial/spatial
EXECUTE SDO_UTIL.PREPARE_FOR_TTS('TBS');
CONNECT SYS/CHANGE_ON_INSTALL AS SYSDBA
EXECUTE DBMS_TTS.TRANSPORT_SET_CHECK('TBS', TRUE);
ALTER TABLESPACE TBS READ ONLY;
EXIT;
EXP USERID = "'SYS/CHANGE_ON_INSTALL AS SYSDBA'" TRANSPORT_TABLESPACE=Y TABLESPACES=TBS FILE=trans_ts.dmp
