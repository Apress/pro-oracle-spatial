-- Listing 12-2. Loading and Location-Enabling the Business Data
$imp spatial/spatial file=app_data.dmp full=y

ALTER TABLE branches ADD (location SDO_GEOMETRY);
UPDATE branches
  SET location = SDO_GCDR.GEOCODE_AS_GEOMETRY (
    'SPATIAL',
    SDO_KEYWORDARRAY (
      street_number || ' ' || street_name,
      city || ' ' || postal_code),
    'US'
  );
COMMIT;

INSERT INTO USER_SDO_GEOM_METADATA (
  TABLE_NAME, COLUMN_NAME, DIMINFO, SRID)
VALUES (
  'BRANCHES',
  'LOCATION',
  SDO_DIM_ARRAY(
    SDO_DIM_ELEMENT('Longitude', -180, 180, .5),
    SDO_DIM_ELEMENT('Latitude', -90, 90, .5)),
  8307);

CREATE INDEX branches_sx ON branches (location)
  indextype is mdsys.spatial_index;
