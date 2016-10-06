-- Listing D-9. Subsetting a GeoRaster Object
DECLARE
  g SDO_GEORASTER;
  b BLOB;
BEGIN
  SELECT georaster INTO g FROM branches WHERE id = 1;
  DBMS_LOB.CREATETEMPORARY(b, true);
  SDO_GEOR.GETRASTERSUBSET
  (
    georaster => g,
    pyramidlevel => 0,
    window => sdo_number_array(0,0,699,899),
    bandnumbers => '0',
    rasterBlob => b
  );
END;
/
