-- Listing D-11. Generating and Populating the Spatial Extent of the georaster Column
DECLARE
  extent SDO_GEOMETRY;
BEGIN
  SELECT SDO_GEOR.GENERATESPATIALEXTENT(a.georaster) INTO extent
  FROM branches a WHERE a.id=1 FOR UPDATE;
  UPDATE branches a SET a.georaster.spatialextent = extent WHERE a.id=1;
COMMIT;
END;
/
