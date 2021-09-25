SELECT name
FROM Trainer t
WHERE t.id IN (
  SELECT leader_id 
  FROM Gym
  WHERE city IN 
  		(SELECT name
         FROM City
         WHERE description = 'Amazon'
         )
  );