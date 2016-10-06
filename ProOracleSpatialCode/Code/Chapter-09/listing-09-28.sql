-- Listing 9-28. Finding the Coverage of Branches Using SDO_AGGR_UNION
SELECT SDO_AGGR_UNION(SDOAGGRTYPE(a.location, 0.5)) coverage
  FROM branches a;
