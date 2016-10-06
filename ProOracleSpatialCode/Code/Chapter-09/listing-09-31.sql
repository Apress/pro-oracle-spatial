-- Listing 9-31. Finding the Coverage of sales_regions Using SDO_AGGR_CONVEXHULL
 SELECT SDO_AGGR_CONVEXHULL(SDOAGGRTYPE(a.geom, 0.5)) coverage
  FROM sales_regions a;
