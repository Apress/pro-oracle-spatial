-- Listing 9-25. Computing the Centroid for the State of New Hampshire
SELECT SDO_GEOM.SDO_CENTROID(a.geom, 0.5) ctrd
  FROM us_states a
 WHERE state_abrv='NH';
