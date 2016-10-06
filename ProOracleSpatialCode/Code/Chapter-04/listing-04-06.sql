-- Listing 4-6: Example of SDO_GTYPE in the location column of  us_states table.
SELECT a.geom.sdo_gtype
FROM us_states a
WHERE state_abrv='TX';
