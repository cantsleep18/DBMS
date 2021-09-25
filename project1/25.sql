SELECT name
FROM Pokemon
WHERE id IN (SELECT e.after_id
            FROM Evolution e
            WHERE e.after_id NOT IN 
            		(SELECT e2.after_id	
                     FROM Evolution e1
                     JOIN Evolution e2 ON e1.after_id = e2.before_id)

			)			
ORDER BY name;