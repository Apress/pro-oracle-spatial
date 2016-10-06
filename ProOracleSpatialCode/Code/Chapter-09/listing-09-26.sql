-- Listing 9-26. Obtaining a Point on the Surface of the Geometry of the State of Massachusetts
SELECT SDO_GEOM.SDO_POINTONSURFACE(a.geom, 0.5) pt
  FROM us_states a
 WHERE state_abrv='MA';
