SELECT DISTINCT name
From Trainer t
INNER JOIN CatchedPokemon cp on t.id = cp.owner_id
WHERE cp.pid IN (SELECT id
                FROM Pokemon
                WHERE type = 'Psychic')
ORDER BY name;