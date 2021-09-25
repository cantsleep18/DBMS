SELECT name
FROM Trainer t
INNER JOIN (SELECT owner_id, count(pid) pid_c, count(distinct pid) pid_dist
            FROM CatchedPokemon cp
            GROUP BY owner_id) a on t.id = a.owner_id
WHERE a.pid_c - a.pid_dist > 0
ORDER BY name;