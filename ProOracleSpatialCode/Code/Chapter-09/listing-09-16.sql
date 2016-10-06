-- Listing 9-16. SDO_DIFFERENCE of Competitor Region 2 with Sales Region 6
CREATE TABLE exclusive_region_for_comp_2 AS
  SELECT SDO_GEOM.SDO_DIFFERENCE(b.geom, a.geom, 0.5) geom
    FROM sales_regions a, competitors_sales_regions b
   WHERE b.id=2
     AND a.id=6 ;
