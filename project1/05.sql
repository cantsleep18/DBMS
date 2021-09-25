SELECT Avg(level)
FROM CatchedPokemon
WHERE owner_id = 
	(SELECT id
     FROM Trainer
     WHERE name = 'Red');