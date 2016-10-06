-- Listing 9-27. Finding the Extent of a Set of Geometries Using SDO_AGGR_MBR
SELECT SDO_AGGR_MBR(a.location) extent
  FROM branches a;
