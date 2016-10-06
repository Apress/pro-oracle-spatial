-- Listing 9-19. SDO_XOR of Sales Regions 43 and 51 to Identify Customers That Are Not Shared Between Them
SELECT count(*)
  FROM (
         SELECT SDO_GEOM.SDO_XOR (a.geom, b.geom, 0.5) geom
           FROM sales_regions b, sales_regions a
          WHERE a.id=51 and b.id=43
       ) b,
       customers a
 WHERE SDO_RELATE(a.location, b.geom, 'mask=anyinteract')='TRUE';
