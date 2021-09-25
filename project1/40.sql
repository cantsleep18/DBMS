SELECT DISTINCT name
FROM Trainer t
INNER JOIN CatchedPokemon cp on t.id = cp.owner_id
WHERE pid IN (SELECT id
             FROM Pokemon
             WHERE name = 'Pikachu')
ORDER BY name;