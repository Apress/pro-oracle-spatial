-- Listing 14-18. Rebuilding All UNUSABLE Indexes for a Table Partition
ALTER TABLE weather_patterns MODIFY PARTITION P1 REBUILD UNUSABLE LOCAL INDEXES;
