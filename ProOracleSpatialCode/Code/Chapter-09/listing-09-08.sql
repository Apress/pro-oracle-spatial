-- Listing 9-8. Identifying Customers in Quarter-Mile Buffer Around a Competitor
col relationship for a20
SELECT ct.id, ct.name,
       SDO_GEOM.RELATE (ct.location, 'DETERMINE', comp.geom, 0.5) relationship
  FROM customers ct, competitors_sales_regions comp
 WHERE comp.id=1
   AND SDO_RELATE(ct.location, comp.geom, 'mask=anyinteract')='TRUE';
