-- Listing 9-24. Computing the Convex Hull for the State of New Hampshire
SELECT SDO_GEOM.SDO_CONVEXHULL(a.geom, 0.5) cvxhl
  FROM us_states a
 WHERE a.state_abrv='NH';
