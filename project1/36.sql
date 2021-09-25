SELECT name
FROM Pokemon p
WHERE id IN(SELECT e1.after_id
            FROM Evolution e1
            LEFT JOIN Evolution e2 ON e1.after_id = e2.before_id
            WHERE e2.before_id is NULL) 
           	AND
            type = 'Water'
ORDER BY name;