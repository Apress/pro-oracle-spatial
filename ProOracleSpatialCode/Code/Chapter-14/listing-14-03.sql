-- Listing 14-3. Nearest-Neighbor Query on the customers Table
SELECT COUNT(*)
  FROM branches a, customers b
 WHERE a.id=1
   AND SDO_NN(b.location, a.location, 'SDO_NUM_RES=100')='TRUE';
