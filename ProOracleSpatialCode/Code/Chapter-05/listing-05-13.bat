REM Listing 5-13. Creating the Transported Tablespace in the Target Database
IMP USERID = "'SYS/CHANGE_ON_INSTALL'" TRANSPORT_TABLESPACE=Y FILE=trans_ts.dmp DATAFILES='sdo_tts.dbf' TABLESPACES=tbs
