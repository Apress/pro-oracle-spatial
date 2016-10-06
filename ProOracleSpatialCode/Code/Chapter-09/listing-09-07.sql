-- Listing 9-7. Identifying Suppliers in a Quarter-Mile Buffer Around a Competitor
SELECT a.id
  FROM suppliers a, competitors_sales_regions b
 WHERE SDO_GEOM.RELATE (a.location, 'ANYINTERACT', b.geom, 0.5) = 'TRUE'
   AND b.id=1;
