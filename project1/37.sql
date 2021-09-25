SELECT SUM(level)
FROM CatchedPokemon cp
WHERE pid In (SELECT id
             FROM Pokemon
             WHERE type = 'Fire');