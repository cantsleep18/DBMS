SELECT Trainer.name, Avg(level)
FROM CatchedPokemon
JOIN Trainer ON Trainer.id = CatchedPokemon.owner_id 
WHERE owner_id IN
	(SELECT leader_id
     FROM Gym)
GROUP BY owner_id
ORDER BY Trainer.name;
