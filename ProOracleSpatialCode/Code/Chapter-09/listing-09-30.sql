-- Listing 9-30. Union of All Sales Regions to Obtain Business Coverage
SELECT SDO_AGGR_UNION(SDOAGGRTYPE(a.geom, 0.5)) coverage
  FROM sales_regions a;
