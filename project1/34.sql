SELECT name
FROM Pokemon
WHERE id IN (SELECT e2.after_id
             FROM Evolution e1
             JOIN Evolution e2 ON e1.after_id = e2.before_id
             JOIN Pokemon p on p.id = e2.before_id
             WHERE e1.before_id = (	SELECT id
                                   	FROM Pokemon
                                   	WHERE name = 'Charmander')
            );