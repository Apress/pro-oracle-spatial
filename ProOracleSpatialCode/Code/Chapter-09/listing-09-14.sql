-- Listing 9-14. SDO_UNION of Two Geometries
SELECT count(*)
  FROM
       (
         SELECT SDO_GEOM.SDO_UNION (a.geom, b.geom, 0.5) geom
         FROM sales_regions b, sales_regions a
         WHERE a.id=51 and b.id=43
       ) b, customers a
 WHERE SDO_RELATE(a.location, b.geom, 'mask=anyinteract')='TRUE';
