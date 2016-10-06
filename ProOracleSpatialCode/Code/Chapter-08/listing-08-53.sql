-- Listing 8-53. Setting Session Parameters to Enable Query Rewrite on Function-Based Indexes
ALTER SESSION SET QUERY_REWRITE_INTEGRITY = TRUSTED;
ALTER SESSION SET QUERY_REWRITE_ENABLED = TRUE;
