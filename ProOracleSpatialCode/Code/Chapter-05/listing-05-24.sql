-- Listing 5-24. Validation Check on a Geometry from the sales_regions Table
SELECT SDO_GEOM.VALIDATE_GEOMETRY_WITH_CONTEXT(a.geom, 0.5) is_valid
FROM sales_regions a
WHERE a.id=10000;
