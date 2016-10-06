-- Listing 9-5. Identifying Customers Inside a Competitor’s Sales Region
SELECT ct.id, ct.name
  FROM customers ct, competitors_sales_regions comp
 WHERE SDO_GEOM.RELATE (ct.location, 'INSIDE', comp.geom, 0.1) = 'INSIDE'
   AND comp.id=1
 ORDER BY ct.id;
