-- Listing D-10. Georeferencing a GeoRaster Object
DECLARE
  g SDO_GEORASTER;
  b BLOB;
BEGIN
  SELECT georaster INTO g FROM branches WHERE id = 1;
  SDO_GEOR.GEOREFERENCE
  (
    georaster => g,
    srid => 8307,
    modelcoordinatelocation => 0, -- 0 for center of the picture
    xCoefficients => sdo_number_array(30, 0, 410000.0), -- values for a, b, and c
    yCoefficients => sdo_number_array(0, -30, 3759000.0) -- values for d, e, and f
  );
  UPDATE branches SET georaster = g WHERE id = 1;
END;
/
