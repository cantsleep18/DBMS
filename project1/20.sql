SELECT name
FROM Trainer t
WHERE t.id IN
	(SELECT leader_id
     FROM Gym
    )
ORDER BY name;