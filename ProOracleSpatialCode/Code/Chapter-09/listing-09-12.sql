-- Listing 9-12. Identifying Customers in sales_ intersection_zones
SELECT count(*)
  FROM sales_intersection_zones b, customers a
 WHERE b.id1=51 AND b.id2=43
   AND SDO_RELATE(a.location, b.intsxn_geom, 'mask=anyinteract')='TRUE';
