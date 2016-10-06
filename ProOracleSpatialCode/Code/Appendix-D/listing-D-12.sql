-- Listing D-12. Populating the Metadata for the Spatial Extent of the georaster Column
INSERT INTO USER_SDO_GEOM_METADATA
VALUES
(
  'BRANCHES',
  'GEORASTER.SPATIALEXTENT',
  SDO_DIM_ARRAY
  (
    SDO_DIM_ELEMENT('X', -180, 180, 0.5),
    SDO_DIM_ELEMENT('Y', -90, 90, 5)
  ),
  8307 -- SRID
);
COMMIT;
