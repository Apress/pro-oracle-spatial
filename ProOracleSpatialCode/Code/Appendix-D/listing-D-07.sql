-- Listing D-7. Granting Permissions to Import Data into a GeoRaster Column
CONNECT system/manager          -- Replace with password for system

-- Grant permission to user 'spatial'
CALL DBMS_JAVA.GRANT_PERMISSION(
  'SPATIAL',
  'SYS:java.io.FilePermission',
  '/usr/rasters/r1.tif',
  'read');

-- Grant permission to the MDSYS schema
CALL DBMS_JAVA.GRANT_PERMISSION(
  'MDSYS',
  'SYS:java.io.FilePermission',
  '/usr/rasters/r1.tif',
  'read');

CONNECT spatial/spatial
