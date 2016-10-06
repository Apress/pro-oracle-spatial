
-- Insert metadata
INSERT INTO USER_SDO_GEOM_METADATA VALUES
('WEATHER_PATTERNS', 'GEOM', 
 SDO_DIM_ARRAY(
   SDO_DIM_ELEMENT('LONG', -180, 180, 0.5),
   SDO_DIM_ELEMENT('LAT', -90, 90, 0.5)
 ),
 8307
);

-- Listing 14-15. Creating a Local Partitioned Spatial Index
CREATE INDEX weather_patterns_sidx ON weather_patterns(geom)
  INDEXTYPE IS MDSYS.SPATIAL_INDEX
  LOCAL;
