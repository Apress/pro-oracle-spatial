-- Listing 9-20. Area of the Intersection Region of Sales Region 43 and Sales Region 51
SELECT SDO_GEOM.SDO_AREA
        (SDO_GEOM.SDO_INTERSECTION(a.geom, b.geom, 0.5), 0.5, ' unit=sq_yard ' ) area
  FROM sales_regions b, sales_regions a
 WHERE a.id=51
   AND b.id=43;
