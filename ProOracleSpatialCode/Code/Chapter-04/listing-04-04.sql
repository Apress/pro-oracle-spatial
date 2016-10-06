-- Listing 4-4. Example of SDO_GTYPE in the geom Column of us_interstates table
SELECT a.geom.sdo_gtype
FROM us_interstates a
WHERE rownum=1;
