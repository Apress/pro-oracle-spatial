-- Listing 8-41. Identifying All Sales Regions That Overlap a Specific Sales Region (id=51)
SELECT a.id
  FROM sales_regions b, sales_regions a
 WHERE b.id=51
   AND a.id <> 51
   AND SDO_RELATE
        (a.geom, b.geom, 'MASK=OVERLAPBDYDISJOINT+OVERLAPBDYINTERSECT')='TRUE' ;
