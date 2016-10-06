-- Listing 8-65. SDO_JOIN Operator Retrieving All Customers Whose MBRs Intersect the MBRs of Competitor Regions (Filter Operation Is Used)
SELECT COUNT(DISTINCT ct.id)
  FROM competitors_sales_regions comp, customers ct,
       TABLE
       (
         SDO_JOIN
         (
           'COMPETITORS_SALES_REGIONS', 'GEOM',   -- first table and column
           'CUSTOMERS', 'LOCATION'                -- second table and column
         )
       ) jn
 WHERE ct.rowid=jn.rowid2
   AND comp.rowid = jn.rowid1;
