-- Listing 4-11. Constructing a Point Geometry Using Well-Known Text (SQL/MM)
SELECT SDO_GEOMETRY(' POINT(-79 37) ', 8307) geom FROM DUAL;
