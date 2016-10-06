-- Listing A-7. Simplifying the Geometry for New Hampshire
SELECT SDO_SAM.SIMPLIFY_GEOMETRY(geom, 0.5)
FROM us_states
WHERE state_abrv='NH';
