SELECT SUM(level)
FROM CatchedPokemon cp
WHERE owner_id IN
			(SELECT id
             FROM Trainer
             WHERE name = 'Matis'
             );
             