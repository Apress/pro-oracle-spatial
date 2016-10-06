-- Listing 8-51. Inserting the Metadata for a <table, function-based pseudo-column>
INSERT INTO user_sdo_geom_metadata VALUES
(
  'CUSTOMERS',
  'SPATIAL.GCDR_GEOMETRY(street_number,street_name,city,state,postal_code)',
  SDO_DIM_ARRAY
  (
    SDO_DIM_ELEMENT('X', -180, 180, 0.5),
    SDO_DIM_ELEMENT('Y', -90, 90, 0.5)
  ),
  8307
);
