-- Listing A-1. Tiling a Two-Dimensional Space
SELECT *
  FROM TABLE (SDO_SAM.TILED_BINS(-77.1027, -76.943996, 38.820813, 38.95911,1, 8307));
