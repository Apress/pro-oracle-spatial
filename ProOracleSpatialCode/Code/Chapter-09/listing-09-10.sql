-- Listing 9-10. SDO_INTERSECTION of Two Geometries
CREATE TABLE sales_intersection_zones AS
  SELECT a.id id1, b.id id2,
         SDO_GEOM.SDO_INTERSECTION(a.geom, b.geom, 0.5) intsxn_geom
    FROM sales_regions b, sales_regions a
   WHERE a.id<> b.id
     AND SDO_RELATE(a.geom, b.geom, 'mask=anyinteract') = 'TRUE' ;
