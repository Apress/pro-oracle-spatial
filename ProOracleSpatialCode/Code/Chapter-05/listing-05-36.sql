-- Listing 5-36. Finding the Number of Elements in a Geometry
SELECT SDO_UTIL.GETNUMELEM(geom) nelem
FROM sales_regions
WHERE id=10000;
