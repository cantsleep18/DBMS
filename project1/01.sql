SELECT name
FROM Trainer
JOIN CatchedPokemon ON Trainer.id = CatchedPokemon.owner_id
Group by name
HAVING count(CatchedPokemon.owner_id) >= 3;