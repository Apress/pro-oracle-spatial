-- Listing 5-29. Example of Removing Duplicate Vertices in a Geometry
SELECT geom, SDO_UTIL.REMOVE_DUPLICATE_VERTICES(a.geom,0.5) nodup_geom
FROM sales_regions a
WHERE id=1000;
