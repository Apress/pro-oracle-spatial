-- Listing D-8. Generating Pyramids for a GeoRaster Object in the branches Table
DECLARE
  geor sdo_georaster;
BEGIN
  SELECT georaster INTO geor FROM branches WHERE id = 1 FOR UPDATE;
  -- Generate four levels of pyramids
  SDO_GEOR.GENERATEPYRAMID(geor, 'rlevel=4');
  UPDATE branches SET georaster = geor WHERE id = 1;
END;
/
