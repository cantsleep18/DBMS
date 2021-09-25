SELECT name, count(pid)
FROM Trainer t
INNER JOIN CatchedPokemon cp on t.id = cp.owner_id
GROUP BY name;