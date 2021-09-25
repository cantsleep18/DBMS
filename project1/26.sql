SELECT name, Sum(level)
FROM Trainer t
INNER JOIN CatchedPokemon cp on t.id = cp.owner_id
GROUP BY name
HAVING Sum(level) = (SELECT Max(sum_lv)
                     FROM Trainer t
                     INNER JOIN (SELECT name, SUM(level) as sum_lv
                                 FROM Trainer t 
                                 INNER JOIN CatchedPokemon cp on t.id = cp.owner_id
                                 GROUP BY name) lv on t.name = lv.name);