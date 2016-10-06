-- Listing 9-22. Computing the MBR of a Geometry
SELECT SDO_GEOM.SDO_MBR(a.geom) mbr
  FROM sales_regions a
 WHERE a.id=1;
