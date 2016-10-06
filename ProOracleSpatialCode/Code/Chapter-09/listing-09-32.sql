-- Listing 9-32. Finding the Centroid of Customer Locations Using SDO_AGGR_CENTROID
SELECT SDO_AGGR_CENTROID(SDOAGGRTYPE(a.location, 0.5)) ctrd
  FROM customers a
 WHERE id<1000;
