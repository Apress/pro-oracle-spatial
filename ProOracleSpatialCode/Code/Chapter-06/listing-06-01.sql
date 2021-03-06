-- Listing 6-1. Geocoding an Address
SELECT SDO_GCDR.GEOCODE_AS_GEOMETRY
(
  'SPATIAL',
  SDO_KEYWORDARRAY
  (
    '3746 CONNECTICUT AVE NW',
    'WASHINGTON, DC 20008'
  ),
  'US'
) geom
FROM DUAL ;
