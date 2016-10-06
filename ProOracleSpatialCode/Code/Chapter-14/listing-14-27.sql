-- Listing 14-27. Determining the SRIDValue in the Location (Geometry) Columns of a Table
SELECT a.location.sdo_srid
FROM customers a
WHERE ROWNUM=1;
