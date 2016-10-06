-- Listing 9-13. Identifying Customers in the Intersection of Sales Regions 51 and 43
SELECT COUNT(*)
  FROM customers ct
 WHERE SDO_RELATE
       (
         ct.location,
         (
           SELECT SDO_GEOM.SDO_INTERSECTION(a.geom, b.geom, 0.5)
           FROM sales_regions a, sales_regions b
           WHERE a.id = 51 and b.id = 43
         ),
         'mask=anyinteract'
       )='TRUE';
