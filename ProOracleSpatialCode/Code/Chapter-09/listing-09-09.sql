-- Listing 9-9. RELATE Function Complementing the SDO_RELATE Operator
SELECT a.id,
       SDO_GEOM.RELATE(a.geom, 'DETERMINE', b.geom, 0.5) relationship
  FROM sales_regions b, sales_regions a
 WHERE b.id=51
   AND a.id<>51
   AND SDO_RELATE
       (
       a.geom,
       b.geom,
       'mask=TOUCH+OVERLAPBDYDISJOINT+OVERLAPBDYINTERSECT'
       ) = 'TRUE'
 ORDER BY a.id;
