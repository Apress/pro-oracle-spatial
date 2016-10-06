-- Listing 8-36. Identifying a DISJOINT relationship
SELECT ct.id, ct.name
  FROM customers ct
 WHERE ct.rowid NOT IN
(
  SELECT ct.rowid
    FROM competitors_sales_regions comp, customers ct
   WHERE comp.id=1
    AND SDO_RELATE(ct.location, comp.geom, 'MASK=ANYINTERACT')='TRUE'
);
