-- Listing D-6. Populating the Georaster Column with a TIFF Image
DECLARE
  g SDO_GEORASTER;
BEGIN
  -- Select the georaster column
  SELECT georaster INTO g FROM branches WHERE id = 1 FOR UPDATE;
  -- Import into the georaster object
  SDO_GEOR.IMPORTFROM
  (
    g, 'blocksize=(512,512)', 'TIFF', 'file',
    '/usr/rasters/r1.tif' -- specify the name and location of the image file
  );
  -- update the column
  UPDATE branches SET georaster = g WHERE id = 1;
END;
/
