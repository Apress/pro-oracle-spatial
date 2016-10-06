REM Listing 10-33. Starting the Network Editor
set JAVA_ORACLE_HOME=D:\Oracle\Ora101
set JAR_LIBS=%JAVA_ORACLE_HOME%/md/lib/sdondme.jar;%JAVA_ORACLE_HOME%/lib/xmlparserv2.jar;%JAVA_ORACLE_HOME%/jdbc/lib/classes12.jar;%JAVA_ORACLE_HOME%\md/lib/sdonm.jar;%JAVA_ORACLE_HOME%/md/lib/sdoapi.jar;%JAVA_ORACLE_HOME%/md/lib/sdoutl.jar
java -Xms512M -Xmx512M -cp %JAR_LIBS% oracle.spatial.network.editor.NetworkEditor
