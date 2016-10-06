-- Listing 9-29. Union of Three Sales Regions (ids 43, 51, and 2)
SELECT SDO_AGGR_UNION(SDOAGGRTYPE(a.geom, 0.5)) union_geom
  FROM sales_regions a
 WHERE id in (51, 43, 2) ;
